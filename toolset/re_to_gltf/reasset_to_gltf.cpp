#include "project.h"
#include "re_common.hpp"
#include "revil/re_asset.hpp"
#include "spike/gltf.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/io/binwritter_stream.hpp"
#include "spike/io/fileinfo.hpp"
#include "spike/uni/motion.hpp"
#include "spike/uni/rts.hpp"
#include "spike/uni/skeleton.hpp"
#include <nlohmann/json.hpp>

std::string_view filters[]{
    ".mot.43$",     ".mot.65$",      ".mot.78$",
    ".mot.458$",    ".motlist.60$",  ".motlist.85$",
    ".motlist.99$", ".motlist.486$", {},
};

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = REAsset2GLTF_DESC " v" REAsset2GLTF_VERSION
                                ", " REAsset2GLTF_COPYRIGHT "Lukas Cone",
    .filters = filters,
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
  size_t WriteTimes(const std::vector<float> &times,
                    const std::vector<uint16> &indices, GLTFStream &stream);

  std::map<size_t, uint32> boneRemaps;
  std::map<int32, std::pair<std::vector<float>, uint32>> timesByFramerate;
  size_t singleKeyAccess;
  int32 commonStream = -1;
};

size_t MOTGLTF::WriteTimes(const std::vector<float> &times,
                           GLTFStream &stream) {
  const float duration = times.back();
  auto [keyAccess, keyIndex] = NewAccessor(stream, 4);
  keyAccess.count = times.size();
  keyAccess.type = gltf::Accessor::Type::Scalar;
  keyAccess.componentType = gltf::Accessor::ComponentType::Float;
  keyAccess.min.push_back(0);
  keyAccess.max.push_back(duration);
  stream.wr.WriteContainer(times);

  return keyIndex;
}

size_t MOTGLTF::WriteTimes(const std::vector<float> &times,
                           const std::vector<uint16> &indices,
                           GLTFStream &stream) {
  const float duration = times[indices.back()];
  auto [keyAccess, keyIndex] = NewAccessor(stream, 4);
  keyAccess.count = indices.size();
  keyAccess.type = gltf::Accessor::Type::Scalar;
  keyAccess.componentType = gltf::Accessor::ComponentType::Float;
  keyAccess.min.push_back(0);
  keyAccess.max.push_back(duration);

  for (auto i : indices) {
    stream.wr.Write(times[i]);
  }

  return keyIndex;
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

  auto [acc, accId] = NewAccessor(CommonStream(), 4);
  singleKeyAccess = accId;
  acc.count = 1;
  acc.max.emplace_back();
  acc.min.emplace_back();
  acc.type = gltf::Accessor::Type::Scalar;
  acc.componentType = gltf::Accessor::ComponentType::Float;
  CommonStream().wr.Write(0.f);
}

void MOTGLTF::ProcessAnimation(const uni::Motion *anim) {
  gltf::Animation animation;
  animation.name = anim->Name();
  auto &timeData = timesByFramerate.at(anim->FrameRate());
  auto &times = timeData.first;
  uint32 keyAccessor = timeData.second;
  const float duration = anim->Duration();
  size_t upperLimit = gltfutils::FindTimeEndIndex(times, duration);
  auto &aniStream = NewStream(anim->Name() + "-data");

  for (auto a : *anim) {
    if (!boneRemaps.contains(a->BoneIndex())) {
      continue;
    }

    auto stripResult = gltfutils::StripValues(times, upperLimit, a.get());
    const size_t numSamples = stripResult.values.size();
    size_t keyAccessIndex = keyAccessor;
    const bool isVec3 = a->TrackType() != uni::MotionTrack::Rotation;

    if (numSamples == 1) {
      keyAccessIndex = singleKeyAccess;
    } else if (numSamples != upperLimit) {
      keyAccessIndex = WriteTimes(times, stripResult.timeIndices, aniStream);
    } else {
      keyAccessIndex = accessors.size();
      accessors.push_back(accessors[keyAccessor]);
      accessors.back().count = upperLimit;
      accessors.back().max.front() = times[stripResult.timeIndices.back()];
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
        i *= 0x7fff;
        i = Vector4A16(_mm_round_ps(i._data, _MM_ROUND_NEAREST));
        SVector4 comp = i.Convert<int16>();
        aniStream.wr.Write(comp);
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
  gltfutils::BoneInfo infos;

  for (auto skel : *skels.get()) {
    for (auto b : *skel) {
      if (boneRemaps.contains(b->Index())) {
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
        size_t parentIndex = boneRemaps.at(parent->Index());
        nodes[parentIndex].children.push_back(index);
        infos.Add(index, value.translation, parentIndex);
      } else {
        scenes.front().nodes.push_back(nodes.size());
        infos.Add(index, value.translation);
      }

      boneRemaps[b->Index()] = index;
      nodes.emplace_back(std::move(bone));
    }
  }

  gltfutils::VisualizeSkeleton(*this, infos);
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

void AppProcessFile(AppContext *ctx) {
  revil::REAsset asset;
  asset.Load(ctx->GetStream());
  MOTGLTF main;
  main.Pipeline(asset);
  BinWritterRef wr(ctx->NewFile(ctx->workingFile.ChangeExtension(".glb")).str);

  main.FinishAndSave(wr, std::string(ctx->workingFile.GetFolder()));
}
