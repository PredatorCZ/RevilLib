#include "datas/binreader_stream.hpp"
#include "datas/binwritter.hpp"
#include "datas/fileinfo.hpp"
#include "gltf.hpp"
#include "project.h"
#include "re_common.hpp"
#include "revil/re_asset.hpp"
#include "uni/motion.hpp"
#include "uni/rts.hpp"
#include "uni/skeleton.hpp"
#include <nlohmann/json.hpp>

static struct {
  // Keys must be float according to standard
  // However it works in blender
  bool compressKeys = false;
} debug;

es::string_view filters[]{
    ".mot.43$",     ".mot.65$",      ".mot.78$",
    ".mot.458$",    ".motlist.60$",  ".motlist.85$",
    ".motlist.99$", ".motlist.486$", {},
};

static AppInfo_s appInfo{
    AppInfo_s::CONTEXT_VERSION,
    AppMode_e::CONVERT,
    ArchiveLoadType::FILTERED,
    REAsset2GLTF_DESC " v" REAsset2GLTF_VERSION ", " REAsset2GLTF_COPYRIGHT
                      "Lukas Cone",
    nullptr,
    filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

class MOTGLTF : GLTF {
public:
  using GLTF::FinishAndSave;

  void MakeKeyBuffers(const uni::MotionsConst &anims);
  void ProcessAnimation(const uni::Motion *anim);
  void ProcessSkeletons(const uni::SkeletonsConst &skels);
  void Pipeline(const revil::REAsset &asset);

private:
  GLTFStream &CommonStream() {
    if (commonStream < 0) {
      auto &newStream = NewStream("common");
      commonStream = newStream.slot;
      return newStream;
    }
    return Stream(commonStream);
  }

  size_t WriteTimes(const std::vector<float> &times, GLTFStream &stream);
  size_t CompressKeys(const std::vector<float> &times, GLTFStream &stream);

  std::map<size_t, uint32> boneRemaps;
  std::map<int32, std::pair<std::vector<float>, uint32>> timesByFramerate;
  size_t singleKeyAccess;
  int32 commonStream = -1;
};

template <class C, class I>
std::vector<C> resample(const std::vector<I> &input, float fraction,
                        uint32 maxbits) {
  std::vector<C> output;
  output.reserve(input.size());

  for (auto &i : input) {
    float frac = (i * fraction);

    if (frac > 1.f) {
      frac = 1.f;
    }

    output.push_back(frac * maxbits);
  }

  return output;
}

bool fltcmp(float f0, float f1, float epsilon = FLT_EPSILON) {
  return (f1 <= f0 + epsilon) && (f1 >= f0 - epsilon);
}

size_t MOTGLTF::CompressKeys(const std::vector<float> &times,
                             GLTFStream &stream) {
  const float duration = times.back();
  auto [keyAccess, keyIndex] = NewAccessor(stream, 2);
  keyAccess.count = times.size();
  keyAccess.type = gltf::Accessor::Type::Scalar;
  keyAccess.normalized = true;
  keyAccess.min.push_back(0);
  keyAccess.max.push_back(duration);

  auto writeData = [&, &keyAccess = keyAccess](auto &containter) {
    using vtype =
        typename std::remove_reference<decltype(containter)>::type::value_type;
    size_t valueSize = sizeof(vtype);
    stream.wr.WriteContainer(containter);
    keyAccess.componentType = valueSize == 2
                                  ? gltf::Accessor::ComponentType::UnsignedShort
                                  : gltf::Accessor::ComponentType::UnsignedByte;
  };

  if (times.size() > 255) {
    auto processed = resample<uint16>(times, 1.f, 0xffff);
    writeData(processed);
  } else {
    auto processed = resample<uint8>(times, 1.f, 0xff);
    const size_t numKeys = processed.size();

    for (size_t i = 0; i < numKeys - 1; i++) {
      uint8 first = processed[i];
      uint8 &next = processed[i + 1];

      if (first == next) {
        next++;
      }
    }

    if (fltcmp(processed.back() * (1.f / 0xff), duration, 0.0001f)) {
      auto processed = resample<uint16>(times, 1.f, 0xffff);
      writeData(processed);
    } else {
      writeData(processed);
    }
  }

  return keyIndex;
}

size_t MOTGLTF::WriteTimes(const std::vector<float> &times,
                           GLTFStream &stream) {
  const float duration = times.back();

  if (debug.compressKeys && duration <= 1.f) {
    return CompressKeys(times, stream);
  }

  auto [keyAccess, keyIndex] = NewAccessor(stream, 4);
  keyAccess.count = times.size();
  keyAccess.type = gltf::Accessor::Type::Scalar;
  keyAccess.componentType = gltf::Accessor::ComponentType::Float;
  keyAccess.min.push_back(0);
  keyAccess.max.push_back(duration);
  stream.wr.WriteContainer(times);

  return keyIndex;
}

struct StripResult {
  std::vector<float> times;
  std::vector<Vector4A16> values;
};

StripResult StripValues(std::vector<float> times, size_t upperLimit,
                        const uni::MotionTrack *tck) {
  StripResult retval;
  retval.times.push_back(times[0]);
  Vector4A16 low, middle;
  tck->GetValue(low, times[0]);
  retval.values.push_back(low);

  if (upperLimit == 1) {
    return retval;
  }

  tck->GetValue(middle, times[1]);

  if (middle == low) {
    return retval;
  }

  retval.times.push_back(times[1]);
  retval.values.push_back(middle);

  for (size_t i = 2; i < upperLimit; i++) {
    Vector4A16 high;
    tck->GetValue(high, times[i]);

    for (size_t p = 0; p < 4; p++) {
      if (!fltcmp(low[p], high[p], 0.00001f)) {
        auto ratio = (low[p] - middle[p]) / (low[p] - high[p]);
        if (!fltcmp(ratio, 0.5f, 0.00001f)) {
          retval.times.push_back(times[i]);
          retval.values.push_back(high);
          break;
        }
      }
    }

    auto tmp = middle;
    middle = high;
    low = tmp;
  }

  return retval;
}

void MOTGLTF::MakeKeyBuffers(const uni::MotionsConst &anims) {
  for (auto a : *anims) {
    auto &maxDuration = timesByFramerate[a->FrameRate()];
    if (maxDuration.first.empty()) {
      maxDuration.first.push_back(a->Duration());
    } else {
      maxDuration.first.front() =
          std::max(maxDuration.first.front(), a->Duration());
    }
  }

  for (auto &[fps, value] : timesByFramerate) {
    auto &[times, keyIndex] = value;
    const float duration = times.front();
    times.pop_back();
    const float fraction = 1.f / fps;
    float cdur = 0;

    while (cdur < duration) {
      times.push_back(cdur);
      cdur += fraction;
    }

    times.back() = duration;

    keyIndex = WriteTimes(times, CommonStream());
  }

  auto [acc, _] = NewAccessor(CommonStream(), 4);
  acc.count = 1;
  acc.max.front() = 0;
}

void MOTGLTF::ProcessAnimation(const uni::Motion *anim) {
  gltf::Animation animation;
  animation.name = anim->Name();
  auto &timeData = timesByFramerate.at(anim->FrameRate());
  auto &times = timeData.first;
  uint32 keyAccessor = timeData.second;
  const float duration = anim->Duration();
  size_t upperLimit = 0;

  for (size_t i = 0; i < times.size(); i++) {
    if (fltcmp(times[i], duration, 0.0001f)) {
      upperLimit = i + 1;
      break;
    }
  }

  if (!upperLimit) {
    throw std::logic_error("Floating point mismatch");
  }

  auto &aniStream = NewStream(anim->Name() + "-data");

  for (auto a : *anim) {
    auto stripResult = StripValues(times, upperLimit, a.get());
    const size_t numSamples = stripResult.values.size();
    size_t keyAccessIndex = keyAccessor;
    const bool isVec3 = a->TrackType() != uni::MotionTrack::Rotation;

    if (numSamples == 1) {
      keyAccessIndex = singleKeyAccess;
    } else if (numSamples != upperLimit) {
      keyAccessIndex = WriteTimes(stripResult.times, aniStream);
    } else {
      keyAccessIndex = accessors.size();
      accessors.push_back(accessors[keyAccessor]);
      accessors.back().count = upperLimit;
      accessors.back().max.front() = stripResult.times.back();
    }

    animation.channels.emplace_back();
    auto &curChannel = animation.channels.back();
    curChannel.sampler = animation.samplers.size();
    curChannel.target.node = boneRemaps.at(a->BoneIndex());

    animation.samplers.emplace_back();
    auto &sampler = animation.samplers.back();
    sampler.input = keyAccessIndex;

    auto [transAccess, transIndex] = NewAccessor(aniStream, 4);
    transAccess.count = stripResult.values.size();
    sampler.output = transIndex;

    if (isVec3) {
      transAccess.componentType = gltf::Accessor::ComponentType::Float;
      transAccess.type = gltf::Accessor::Type::Vec3;

      for (auto &i : stripResult.values) {
        aniStream.wr.Write(Vector(i));
      }
    } else {
      transAccess.componentType = gltf::Accessor::ComponentType::Short;
      transAccess.normalized = true;
      transAccess.type = gltf::Accessor::Type::Vec4;

      for (auto &i : stripResult.values) {
        aniStream.wr.Write(Vector4A16(i * 32767).Convert<int16>());
      }
    }

    switch (a->TrackType()) {
    case uni::MotionTrack::Position:
      curChannel.target.path = "translation";
      break;
    case uni::MotionTrack::Rotation:
      curChannel.target.path = "rotation";
      break;
    case uni::MotionTrack::Scale:
      curChannel.target.path = "scale";
      break;
    default:
      break;
    }
  }

  animations.emplace_back(std::move(animation));
}

void MOTGLTF::ProcessSkeletons(const uni::SkeletonsConst &skels) {
  skins.emplace_back();
  auto &skin = skins.back();

  for (auto skel : *skels.get()) {
    for (auto b : *skel) {
      if (boneRemaps.count(b->Index())) {
        continue;
      }

      gltf::Node bone;
      uni::RTSValue value;
      b->GetTM(value);
      memcpy(bone.translation.data(), &value.translation,
             sizeof(bone.translation));
      memcpy(bone.rotation.data(), &value.rotation, sizeof(bone.rotation));
      memcpy(bone.scale.data(), &value.scale, sizeof(bone.scale));
      bone.name = b->Name();
      auto parent = b->Parent();
      auto index = nodes.size();
      int32 hash = b->Index();
      bone.GetExtensionsAndExtras()["extras"]["hash"] = hash;

      if (parent) {
        nodes[boneRemaps.at(parent->Index())].children.push_back(index);
      }

      boneRemaps[b->Index()] = index;
      skin.joints.push_back(index);
      nodes.emplace_back(std::move(bone));
    }
  }
}

void MOTGLTF::Pipeline(const revil::REAsset &asset) {
  auto skels = asset.As<uni::SkeletonsConst>();
  ProcessSkeletons(skels);
  auto anims = asset.As<uni::MotionsConst>();
  MakeKeyBuffers(anims);

  for (auto a : *anims) {
    ProcessAnimation(a.get());
  }
}

void AppProcessFile(std::istream &stream, AppContext *ctx) {
  revil::REAsset asset;
  BinReaderRef rd(stream);
  asset.Load(rd);
  MOTGLTF main;
  main.Pipeline(asset);
  AFileInfo outPath(ctx->outFile);
  BinWritter wr(outPath.GetFullPathNoExt().to_string() + ".glb");

  main.FinishAndSave(wr, outPath.GetFolder());
}
