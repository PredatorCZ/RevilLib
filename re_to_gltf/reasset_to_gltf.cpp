#include "datas/binwritter_stream.hpp"
#include "datas/directory_scanner.hpp"
#include "datas/fileinfo.hpp"
#include "datas/multi_thread.hpp"
#include "datas/pugiex.hpp"
#include "datas/reflector_xml.hpp"
#include "datas/settings_manager.hpp"
#include "datas/stat.hpp"
#include "datas/tchar.hpp"
#include "gltf.h"
#include "project.h"
#include "revil/re_asset.hpp"
#include "uni/motion.hpp"
#include "uni/rts.hpp"
#include "uni/skeleton.hpp"
#include <sstream>

using json = nlohmann::json;
using namespace fx;

static struct {
  bool binaryGLTF = false;
  // Keys must be float according to standard
  // However it works in blender
  bool compressKeys = false;
} debug;

static struct REAsset2GLTF : SettingsManager<REAsset2GLTF> {
  bool Generate_Log = false;
} settings;

REFLECTOR_CREATE(REAsset2GLTF, 1, EXTENDED,
                 (D, Generate_Log,
                  "Will generate text log of console output next to "
                  "application location."));

static const char appHeader[] = REAsset2GLTF_DESC
    " v" REAsset2GLTF_VERSION ", " REAsset2GLTF_COPYRIGHT "Lukas Cone"
    "\nSimply drag'n'drop files/folders onto application or "
    "use as " REAsset2GLTF_NAME
    " path1 path2 ...\nTool can detect and scan folders.";

static const char configHelp[] = "For settings, edit .config file.";

class GLTF {
public:
  std::map<size_t, uint32> boneRemaps;
  std::map<int32, std::pair<std::vector<float>, uint32>> timesByFramerate;
  std::stringstream str;
  BinWritterRef wr;
  gltf::Document doc;
  size_t singleKeyAccess;

  GLTF();
  void MakeKeyBuffers(const uni::MotionsConst &anims);
  void ProcessAnimation(const uni::Motion *anim);
  void ProcessSkeletons(const uni::SkeletonsConst &skels);
  void Pipeline(const revil::REAsset &asset);
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

size_t CompressKeys(const std::vector<float> &times, BinWritterRef wr,
                    gltf::Document &doc) {
  const float duration = times.back();
  size_t accessIndex = doc.accessors.size();

  doc.accessors.emplace_back();
  auto &keyAccess = doc.accessors.back();
  keyAccess.bufferView = doc.bufferViews.size();
  keyAccess.count = times.size();
  keyAccess.type = gltf::Accessor::Type::Scalar;
  keyAccess.normalized = true;
  keyAccess.min.push_back(0);
  keyAccess.max.push_back(duration);

  auto writeData = [&](auto &containter) {
    using vtype =
        typename std::remove_reference<decltype(containter)>::type::value_type;
    size_t valueSize = sizeof(vtype);
    wr.WriteContainer(containter);
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

  return accessIndex;
}

size_t WriteTimes(const std::vector<float> &times, BinWritterRef wr,
                  gltf::Document &doc) {
  const float duration = times.back();
  size_t accessIndex = doc.accessors.size();

  if (debug.compressKeys && duration <= 1.f) {
    CompressKeys(times, wr, doc);
  } else {
    doc.accessors.emplace_back();
    auto &keyAccess = doc.accessors.back();
    keyAccess.bufferView = doc.bufferViews.size();
    keyAccess.count = times.size();
    keyAccess.type = gltf::Accessor::Type::Scalar;
    keyAccess.componentType = gltf::Accessor::ComponentType::Float;
    keyAccess.min.push_back(0);
    keyAccess.max.push_back(duration);

    wr.WriteContainer(times);
  }

  wr.ApplyPadding();

  return accessIndex;
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

GLTF::GLTF() : wr(str) {
  doc.scenes.emplace_back();
  doc.scenes.back().nodes.push_back(0);
}

void GLTF::MakeKeyBuffers(const uni::MotionsConst &anims) {
  for (auto a : *anims) {
    auto &maxDuration = timesByFramerate[a->FrameRate()];
    if (maxDuration.first.empty()) {
      maxDuration.first.push_back(a->Duration());
    } else {
      maxDuration.first.front() =
          std::max(maxDuration.first.front(), a->Duration());
    }
  }

  for (auto &d : timesByFramerate) {
    auto &times = d.second.first;
    const float duration = times.front();
    times.pop_back();
    const float fraction = 1.f / d.first;
    float cdur = 0;

    while (cdur < duration) {
      times.push_back(cdur);
      cdur += fraction;
    }

    times.back() = duration;

    size_t keyAccessor = WriteTimes(times, wr, doc);
    d.second.second = keyAccessor;
  }

  singleKeyAccess = doc.accessors.size();
  doc.accessors.push_back(doc.accessors.back());
  doc.accessors.back().count = 1;
}

void GLTF::ProcessAnimation(const uni::Motion *anim) {
  gltf::Animation animation;
  animation.name = anim->Name();
  auto &timeData = timesByFramerate.at(anim->FrameRate());
  auto &times = timeData.first;
  uint32 keyAccessor = timeData.second;
  const float duration = anim->Duration();
  size_t upperLimit = 0;

  for (size_t i = 0; i < times.size(); i++) {
    if (fltcmp(times[i], duration, 0.00001f)) {
      upperLimit = i + 1;
      break;
    }
  }

  if (!upperLimit) {
    throw std::logic_error("Floating point mismatch");
  }

  for (auto a : *anim) {
    auto stripResult = StripValues(times, upperLimit, a.get());
    const size_t numSamples = stripResult.values.size();
    size_t keyAccessIndex = keyAccessor;
    const bool isVec3 = a->TrackType() != uni::MotionTrack::Rotation;

    if (numSamples == 1) {
      keyAccessIndex = singleKeyAccess;
    } else if (numSamples != upperLimit) {
      keyAccessIndex = WriteTimes(stripResult.times, wr, doc);
    } else {
      keyAccessIndex = doc.accessors.size();
      doc.accessors.push_back(doc.accessors[keyAccessor]);
      doc.accessors.back().count = upperLimit;
    }

    animation.samplers.emplace_back();
    auto &sampler = animation.samplers.back();
    sampler.input = keyAccessIndex;
    sampler.output = doc.accessors.size();

    animation.channels.emplace_back();
    auto &curChannel = animation.channels.back();
    curChannel.sampler = animation.samplers.size();
    curChannel.target.node = boneRemaps.at(a->BoneIndex());

    doc.accessors.emplace_back();
    auto &transAccess = doc.accessors.back();
    transAccess.bufferView = doc.bufferViews.size();
    transAccess.count = stripResult.values.size();
    transAccess.byteOffset = wr.Tell();

    if (isVec3) {
      transAccess.componentType = gltf::Accessor::ComponentType::Float;
      transAccess.type = gltf::Accessor::Type::Vec3;

      for (auto &i : stripResult.values) {
        wr.Write(Vector(i));
      }
    } else {
      transAccess.componentType = gltf::Accessor::ComponentType::Short;
      transAccess.normalized = true;
      transAccess.type = gltf::Accessor::Type::Vec4;

      for (auto &i : stripResult.values) {
        wr.Write(Vector4A16(i * 32767).Convert<int16>());
      }
    }

    wr.ApplyPadding();

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

  doc.animations.emplace_back(std::move(animation));
}

void GLTF::ProcessSkeletons(const uni::SkeletonsConst &skels) {
  auto skel = skels->At(0);
  doc.skins.emplace_back();
  auto &skin = doc.skins.back();

  for (auto b : *skel) {
    gltf::Node bone;
    uni::RTSValue value;
    b->GetTM(value);
    memcpy(bone.translation.data(), &value.translation,
           sizeof(bone.translation));
    memcpy(bone.rotation.data(), &value.rotation, sizeof(bone.rotation));
    memcpy(bone.scale.data(), &value.scale, sizeof(bone.scale));
    bone.name = b->Name();
    auto parent = b->Parent();
    auto index = doc.nodes.size();
    int32 hash = b->Index();
    bone.extensionsAndExtras["extras"]["hash"] = hash;

    if (parent) {
      doc.nodes[boneRemaps.at(parent->Index())].children.push_back(index);
    }

    boneRemaps[b->Index()] = index;
    skin.joints.push_back(index);
    doc.nodes.emplace_back(std::move(bone));
  }
}

void GLTF::Pipeline(const revil::REAsset &asset) {
  const size_t cBegin = wr.Tell();
  wr.SetRelativeOrigin(cBegin);
  auto skels = asset.As<uni::SkeletonsConst>();
  ProcessSkeletons(skels);
  auto anims = asset.As<uni::MotionsConst>();
  MakeKeyBuffers(anims);

  doc.bufferViews.emplace_back();
  auto &cBuffer = doc.bufferViews.back();
  cBuffer.buffer = doc.buffers.size();
  cBuffer.byteLength = wr.Tell();
  cBuffer.byteOffset = cBegin;
  cBuffer.name = "common_data";
  wr.ResetRelativeOrigin(false);

  for (auto a : *anims) {
    const size_t aBegin = wr.Tell();
    wr.SetRelativeOrigin(aBegin);
    ProcessAnimation(a.get());

    doc.bufferViews.emplace_back();
    auto &aBuffer = doc.bufferViews.back();
    aBuffer.buffer = doc.buffers.size();
    aBuffer.byteLength = wr.Tell();
    aBuffer.byteOffset = aBegin;
    aBuffer.name = a->Name() + "-data";
    wr.ResetRelativeOrigin(false);
  }

  doc.buffers.emplace_back();
  auto &mainBuffer = doc.buffers.back();
  mainBuffer.byteLength = wr.Tell();
  auto buffer = str.str();
  { es::Dispose(str); }
  mainBuffer.data.resize(mainBuffer.byteLength);
  memcpy(mainBuffer.data.data(), buffer.data(), mainBuffer.byteLength);
  if (!debug.binaryGLTF) {
    mainBuffer.SetEmbeddedResource();
  }
}

void ProcessFile(const std::string &fileName) {
  printline("Converting " << fileName);
  revil::REAsset asset;
  asset.Load(fileName);
  GLTF main;
  main.Pipeline(asset);
  AFileInfo info(fileName);
  auto outFile = info.GetFullPathNoExt().to_string() +
                 (debug.binaryGLTF ? ".glb" : ".gltf");
  gltf::Save(main.doc, outFile, debug.binaryGLTF);
}

int _tmain(int argc, TCHAR *argv[]) {
  setlocale(LC_ALL, "");
  printer.AddPrinterFunction(UPrintf);
  printline(appHeader);

  AFileInfo configInfo(std::to_string(*argv));
  auto configName = configInfo.GetFullPathNoExt().to_string() + ".config";
  try {
    auto doc = XMLFromFile(configName);
    ReflectorXMLUtil::LoadV2(settings, doc, true);
  } catch (const es::FileNotFoundError &e) {
  }
  {
    pugi::xml_document doc = {};
    std::stringstream str;
    settings.GetHelp(str);
    auto buff = str.str();
    doc.append_child(pugi::node_comment).set_value(buff.data());

    ReflectorXMLUtil::SaveV2a(settings, doc,
                              {ReflectorXMLUtil::Flags_ClassNode,
                               ReflectorXMLUtil::Flags_StringAsAttribute});
    XMLToFile(configName, doc,
              {XMLFormatFlag::WriteBOM, XMLFormatFlag::IndentAttributes});
  }

  if (argc < 2) {
    printerror("Insufficient argument count, expected at least 1.");
    printline(configHelp);
    return 1;
  }

  if (IsHelp(argv[1])) {
    printline(configHelp);
    return 0;
  }

  if (settings.Generate_Log) {
    settings.CreateLog(configInfo.GetFullPathNoExt().to_string());
  }

  std::vector<std::string> files;

  for (int a = 1; a < argc; a++) {
    auto fileName = std::to_string(argv[a]);
    auto type = FileType(fileName);

    switch (type) {
    case FileType_e::Directory: {
      DirectoryScanner sc;
      sc.AddFilter(".mot.43");
      sc.AddFilter(".mot.65");
      sc.AddFilter(".mot.78");
      sc.AddFilter(".mot.458");
      sc.AddFilter(".motlist.60");
      sc.AddFilter(".motlist.85");
      sc.AddFilter(".motlist.99");
      sc.AddFilter(".motlist.486");
      printline("Scanning: " << fileName);
      sc.Scan(fileName);
      printline("Files found: " << sc.Files().size());

      std::transform(std::make_move_iterator(sc.begin()),
                     std::make_move_iterator(sc.end()),
                     std::back_inserter(files),
                     [](auto &&item) { return std::move(item); });

      break;
    }
    case FileType_e::File:
      files.emplace_back(std::move(fileName));
      break;
    default: {
      printerror("Invalid path: " << fileName);
      break;
    }
    }
  }

  printer.PrintThreadID(true);

  RunThreadedQueue(files.size(), [&](size_t index) {
    try {
      ProcessFile(files[index]);
    } catch (const std::exception &e) {
      printerror(e.what());
    }
  });

  return 0;
}
