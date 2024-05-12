/*  LMT2GLTF
    Copyright(C) 2021-2022 Lukas Cone

    This program is free software : you can redistribute it and / or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.If not, see <https://www.gnu.org/licenses/>.
*/

#include "project.h"
#include "re_common.hpp"
#include "revil/lmt.hpp"
#include "spike/gltf.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/io/binwritter_stream.hpp"
#include "spike/io/fileinfo.hpp"
#include "spike/master_printer.hpp"
#include <set>

#include "animengine.hpp"

#if 0
#include "nlohmann/json.hpp"
using ReportType = nlohmann::json;
#define USE_REPORT
#else
using ReportType = uint64;
#endif

std::string_view filters[]{
    ".lmt$",
    ".bin$",
};

std::string_view controlFilters[]{
    ".glb$",
    ".gltf$",
};

static const float SCALE = 0.01;

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = LMT2GLTF_DESC " v" LMT2GLTF_VERSION ", " LMT2GLTF_COPYRIGHT
                            "Lukas Cone",
    .filters = filters,
    .batchControlFilters = controlFilters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

struct LMTGLTF : GLTF {
  using GLTF::GLTF;

  GLTFStream &AnimStream() {
    if (aniStream < 0) {
      auto &newStream = NewStream("anims");
      aniStream = newStream.slot;
      return newStream;
    }
    return Stream(aniStream);
  }

  std::set<uint32> animatedNodes;
  std::set<uint32> missingBones;

private:
  int32 aniStream = -1;
};

void CreateEffectorNodes(AnimEngine &eng, LMTGLTF &main) {
  for (auto &[id, node] : eng.nodes) {
    int32 nodeId = id;

    if (nodeId > -2) {
      continue;
    }

    std::string nodeName = "ik_" + std::to_string(-nodeId);

    for (size_t curNode = 0; auto &n : main.nodes) {
      if (n.name == nodeName) {
        nodeName.clear();
        node.glNodeIndex = curNode;
        break;
      }

      curNode++;
    }

    if (nodeName.empty()) {
      continue;
    }

    node.glNodeIndex = main.nodes.size();
    gltf::Node &gNode = main.nodes.emplace_back();
    gNode.name = nodeName;
    const size_t parentNode = eng.nodes.at(node.parentAnimNode).glNodeIndex;
    main.nodes.at(parentNode).children.emplace_back(node.glNodeIndex);
    gNode.children.emplace_back(main.nodes.size());
    gltf::Node &dNode = main.nodes.emplace_back();
    dNode.name = nodeName + "_end";
    memcpy(dNode.translation.data(), &node.refPosition, 12);
    main.animatedNodes.emplace(node.glNodeIndex);
    main.animatedNodes.emplace(node.glNodeIndex + 1);
  }
}

gltfutils::StripResult StripValues(std::span<Vector4A16> tck) {
  gltfutils::StripResult retval;
  retval.timeIndices.push_back(0);
  Vector4A16 high;
  Vector4A16 low = tck.front();
  retval.values.push_back(low);

  if (tck.size() == 1) {
    return retval;
  }

  Vector4A16 middle = tck[1];

  for (size_t i = 2; i < tck.size(); i++) {
    high = tck[i];

    for (size_t p = 0; p < 3; p++) {
      if (!gltfutils::fltcmp(low[p], high[p], 0.0001f)) {
        auto ratio = (low[p] - middle[p]) / (low[p] - high[p]);
        if (!gltfutils::fltcmp(ratio, 0.5f, 0.005f)) {
          retval.timeIndices.push_back(i - 1);
          retval.values.push_back(middle);
          break;
        }
      }
    }

    auto tmp = middle;
    middle = high;
    low = tmp;
  }

  if (middle != low) {
    retval.timeIndices.push_back(tck.size() - 1);
    retval.values.push_back(middle);
  }

  return retval;
}

struct StripResult {
  std::vector<uint16> timeIndices;
  std::vector<SVector4> values;
};

StripResult StripValues(std::span<SVector4> tck) {
  StripResult retval;
  retval.timeIndices.push_back(0);
  Vector4A16 high;
  Vector4A16 low = Unpack(tck.front());
  retval.values.push_back(Pack(low));

  if (tck.size() == 1) {
    return retval;
  }

  Vector4A16 middle = Unpack(tck[1]);

  for (size_t i = 2; i < tck.size(); i++) {
    high = Unpack(tck[i]);

    for (size_t p = 0; p < 3; p++) {
      if (!gltfutils::fltcmp(low[p], high[p], 0.0001f)) {
        auto ratio = (low[p] - middle[p]) / (low[p] - high[p]);
        if (!gltfutils::fltcmp(ratio, 0.5f, 0.005f)) {
          retval.timeIndices.push_back(i - 1);
          retval.values.push_back(Pack(middle));
          break;
        }
      }
    }

    auto tmp = middle;
    middle = high;
    low = tmp;
  }

  if (middle != low) {
    retval.timeIndices.push_back(tck.size() - 1);
    retval.values.push_back(Pack(middle));
  }

  return retval;
}

void DumpAnim(AnimEngine &eng, LMTGLTF &main, std::string animName,
              std::span<float> times, uint32 loopFrame, ReportType &) {

  auto TryStripWrite = [&](auto valuesSpan, size_t keys, GLTFStream &stream,
                           size_t accId) {
    auto strip = StripValues(valuesSpan);
    const float stripRatio =
        float(strip.timeIndices.size()) / float(valuesSpan.size());

    if (stripRatio < 0.75f) {
      if constexpr (std::is_same_v<typename decltype(valuesSpan)::value_type,
                                   Vector4A16>) {
        for (auto v : strip.values) {
          stream.wr.Write(Vector(v));
        }
      } else {
        stream.wr.WriteContainer(strip.values);
      }

      auto [keyAccess, keyIndex] = main.NewAccessor(stream, 4);
      keyAccess.type = gltf::Accessor::Type::Scalar;
      keyAccess.componentType = gltf::Accessor::ComponentType::Float;
      keyAccess.min.push_back(times[strip.timeIndices.front()]);
      keyAccess.max.push_back(times[strip.timeIndices.back()]);
      keyAccess.count = strip.timeIndices.size();
      main.accessors.at(accId).count = strip.timeIndices.size();

      for (auto t : strip.timeIndices) {
        stream.wr.Write(times[t]);
      }

      return keyIndex;
    }

    if constexpr (std::is_same_v<typename decltype(valuesSpan)::value_type,
                                 Vector4A16>) {
      for (auto v : valuesSpan) {
        stream.wr.Write(Vector(v));
      }
    } else {
      stream.wr.WriteContainer(valuesSpan);
    }
    return keys;
  };

  auto Write = [&](size_t start, size_t size, size_t keys,
                   std::string animName) {
    gltf::Animation animation;
    animation.name = std::move(animName);

#ifdef USE_REPORT
    auto [animReport, _] = report.emplace(animation.name, ReportType::object());
    auto [channels, _1] = animReport->emplace("channels", ReportType::object());
#endif

    for (auto &[_, node] : eng.nodes) {
      if (node.glNodeIndex < 0) {
        continue;
      }
#ifdef USE_REPORT
      auto [channel, _2] = channels->emplace(
          std::to_string(animation.channels.size()), nlohmann::json::object());
#endif
      if (!node.positions.empty()) {
        animation.channels.emplace_back();
        auto &curChannel = animation.channels.back();
        curChannel.sampler = animation.samplers.size();
        curChannel.target.node = node.glNodeIndex;
        curChannel.target.path = "translation";
#ifdef USE_REPORT
        *channel = node.positionCompression;
#endif
        animation.samplers.emplace_back();
        auto &sampler = animation.samplers.back();

        auto &stream = main.AnimStream();
        auto [transAccess, transIndex] = main.NewAccessor(stream, 4);
        transAccess.count = size;
        sampler.output = transIndex;
        transAccess.componentType = gltf::Accessor::ComponentType::Float;
        transAccess.type = gltf::Accessor::Type::Vec3;

        std::span<Vector4A16> positionsSpan(node.positions);
        positionsSpan = positionsSpan.subspan(start, size);

        sampler.input = TryStripWrite(positionsSpan, keys, stream, transIndex);
      }

      if (!node.rotations.empty()) {
        animation.channels.emplace_back();
        auto &curChannel = animation.channels.back();
        curChannel.sampler = animation.samplers.size();
        curChannel.target.node = node.glNodeIndex;
        curChannel.target.path = "rotation";
#ifdef USE_REPORT
        *channel = node.rotationCompression;
#endif
        animation.samplers.emplace_back();
        auto &sampler = animation.samplers.back();

        auto &stream = main.AnimStream();
        auto [transAccess, transIndex] = main.NewAccessor(stream, 4);
        transAccess.count = size;
        sampler.output = transIndex;
        transAccess.componentType = gltf::Accessor::ComponentType::Short;
        transAccess.normalized = true;
        transAccess.type = gltf::Accessor::Type::Vec4;

        std::span<SVector4> rotationsSpan(node.rotations);
        rotationsSpan = rotationsSpan.subspan(start, size);
        sampler.input = TryStripWrite(rotationsSpan, keys, stream, transIndex);
      }

      if (!node.scales.empty()) {
        animation.channels.emplace_back();
        auto &curChannel = animation.channels.back();
        curChannel.sampler = animation.samplers.size();
        curChannel.target.node = node.glScaleNodeIndex < 0
                                     ? node.glNodeIndex
                                     : node.glScaleNodeIndex;
        curChannel.target.path = "scale";
#ifdef USE_REPORT
        *channel = node.scaleCompression;
#endif
        animation.samplers.emplace_back();
        auto &sampler = animation.samplers.back();

        auto &stream = main.AnimStream();
        auto [transAccess, transIndex] = main.NewAccessor(stream, 4);
        transAccess.count = size;
        sampler.output = transIndex;
        transAccess.componentType = gltf::Accessor::ComponentType::Float;
        transAccess.type = gltf::Accessor::Type::Vec3;

        std::span<Vector4A16> scalesSpan(node.scales);
        scalesSpan = scalesSpan.subspan(start, size);

        sampler.input = TryStripWrite(scalesSpan, keys, stream, transIndex);
      }
    }

    if (animation.channels.size() > 0) {
      main.animations.emplace_back(std::move(animation));
    }
  };

  if (loopFrame) {
    uint32 maxKeys = std::max(loopFrame, uint32(times.size()) - loopFrame);
    std::span<float> timesSpan(times.data(), maxKeys);

    size_t keyIndexStart = [&] {
      auto &stream = main.AnimStream();
      auto [keyAccess, keyIndex] = main.NewAccessor(stream, 4);
      keyAccess.type = gltf::Accessor::Type::Scalar;
      keyAccess.componentType = gltf::Accessor::ComponentType::Float;
      keyAccess.min.push_back(times.front());
      keyAccess.max.push_back(times[loopFrame - 1]);
      keyAccess.count = loopFrame;
      stream.wr.WriteContainer(timesSpan);

      return keyIndex;
    }();

    Write(0, loopFrame, keyIndexStart, animName + "_start");

    const uint32 loopSize = times.size() - loopFrame;
    size_t keyIndexLoop = [&] {
      size_t keyIndex = main.accessors.size();
      auto &keyAccess = main.accessors.emplace_back();
      keyAccess = main.accessors.at(keyIndexStart);
      keyAccess.type = gltf::Accessor::Type::Scalar;
      keyAccess.max.front() = times[loopSize - 1];
      keyAccess.count = loopSize;

      return keyIndex;
    }();

    Write(loopFrame, loopSize, keyIndexLoop, animName + "_loop");
  } else {
    size_t keyIndex = [&] {
      auto &stream = main.AnimStream();
      auto [keyAccess, keyIndex] = main.NewAccessor(stream, 4);
      keyAccess.type = gltf::Accessor::Type::Scalar;
      keyAccess.componentType = gltf::Accessor::ComponentType::Float;
      keyAccess.min.push_back(times[loopFrame]);
      keyAccess.max.push_back(times.back());
      keyAccess.count = times.size();
      stream.wr.WriteContainer(times);

      return keyIndex;
    }();

    Write(loopFrame, times.size(), keyIndex, animName);
  }
}

void DoLmt(LMTGLTF &main, LMT &lmt, std::string name, ReportType &report) {
  int32 sampleRate = 60;
  const float sampleFrac = 1.f / sampleRate;
  uni::MotionsConst motion = lmt;
  IkChainDescripts iks{};

  auto found = IK_VERSION.find(uint16(lmt.Version()));

  if (found != IK_VERSION.end()) {
    iks = found->second;
  }

  for (size_t motionIndex = 0; auto m : *motion) {
    if (!m) {
      motionIndex++;
      continue;
    }

    m->FrameRate(sampleRate);
    auto times = gltfutils::MakeSamples(sampleRate, m->Duration());
    auto lm = static_cast<const LMTAnimation *>(m.get());

    AnimEngine engine;
    LinkNodes(engine, main);
    engine.numSamples = times.size();

    for (auto t : *m) {
      size_t index = t->BoneIndex();

      if (!engine.nodes.contains(index)) {
        main.missingBones.emplace(index);
        continue;
      }

      AnimNode &aNode = engine.nodes.at(index);
      auto tm = static_cast<const LMTTrack *>(t.get());
      aNode.boneType = tm->BoneType();

      if (aNode.boneType
          && !(iks.size() > 0 && iks[aNode.boneType])
      ) {
        PrintLine("M: ", motionIndex, " Index: ", index,
                  " type: ", int(aNode.boneType));
      }

      switch (t->TrackType()) {
      case uni::MotionTrack::Position:
        if (aNode.positions.size() > 0) {
          PrintWarning("Position track already loaded!");
          break;
        }
        aNode.positions.reserve(times.size());
        aNode.positionCompression = tm->CompressionType();

        for (auto k : times) {
          Vector4A16 value;
          t->GetValue(value, k);
          value *= SCALE;
          aNode.positions.emplace_back(value);
        }
        break;
      case uni::MotionTrack::Rotation:
        if (aNode.rotations.size() > 0) {
          PrintWarning("Rotation track already loaded!");
          break;
        }
        aNode.rotations.reserve(times.size());
        aNode.rotationCompression = tm->CompressionType();

        for (auto k : times) {
          Vector4A16 value;
          t->GetValue(value, k);
          aNode.rotations.emplace_back(Pack(value));
        }
        break;
      case uni::MotionTrack::Scale:
        if (aNode.scales.size() > 0) {
          PrintWarning("Scale track already loaded!");
          break;
        }
        aNode.scales.reserve(times.size());
        aNode.scaleCompression = tm->CompressionType();

        for (auto k : times) {
          Vector4A16 value;
          t->GetValue(value, k);
          aNode.scales.emplace_back(value);
        }
        break;
      default:
        break;
      }
    }

    InheritScales(engine, size_t(-1));
    if (iks.size() > 0) {
      SetupChains(engine, iks);
      CreateEffectorNodes(engine, main);
    }

    float loopTime = lm->LoopFrame() * sampleFrac;
    std::string animName = name + "[" + std::to_string(motionIndex) + "]";

    if (loopTime == 0.f) {
      animName.append("_loop");
    }

    uint32 loopFrame = [&]() -> uint32 {
      if (loopTime > 0.f) {
        return gltfutils::FindTimeEndIndex(times, loopTime);
      }

      return 0;
    }();

    DumpAnim(engine, main, animName, times, loopFrame, report);

    motionIndex++;

    for (auto &[id, node] : engine.nodes) {
      main.animatedNodes.emplace(node.glNodeIndex);
      main.animatedNodes.emplace(node.glScaleNodeIndex);
    }
  }
}

void AppProcessFile(AppContext *ctx) {
  LMTGLTF main(gltf::LoadFromBinary(ctx->GetStream(), ""));

  auto &lmts = ctx->SupplementalFiles();
#ifdef USE_REPORT
  nlohmann::json report(nlohmann::json::object());
#else
  ReportType report = 0;
#endif

  for (auto &lmtFile : lmts) {
    auto lmtStream = ctx->RequestFile(lmtFile);
    LMT lmt;
    lmt.Load(*lmtStream.Get());
    DoLmt(main, lmt,
          lmts.size() > 1 ? std::string(AFileInfo(lmtFile).GetFilename())
                          : "Motion",
          report);
  }

  // Create single armature object for easier NLA handle
  gltf::Skin &sharedSkin = main.skins.emplace_back();
  sharedSkin.name = "Animation Group";
  for (uint32 n : main.animatedNodes) {
    if (n > 0xfffff) {
      continue;
    }
    sharedSkin.joints.emplace_back(n);
  }

  for (uint32 id : main.missingBones) {
    PrintLine("Missing bone: ", id);
  }

#ifdef USE_REPORT
  std::set<uint32> animData;

  for (auto &a : main.animations) {
    for (auto &s : a.samplers) {
      animData.emplace(s.output);
    }
  }

  size_t v3Size = 0;
  size_t v4Size = 0;

  for (auto f : animData) {
    auto &acc = main.accessors.at(f);

    if (acc.type == gltf::Accessor::Type::Vec4) {
      v4Size += acc.count * 8;
    } else {
      v3Size += acc.count * 12;
    }
  }

  printline("Rotations buffer size: " << v4Size);
  printline("Translations/Scales buffer size: " << v3Size);

  {
    BinWritterRef wrj(
        ctx->NewFile(std::string(ctx->workingFile.GetFullPathNoExt()) +
                     "_report.json")
            .str);

    wrj.BaseStream() << report;
  }
#endif

  BinWritterRef wr(
      ctx->NewFile(std::string(ctx->workingFile.GetFullPathNoExt()) +
                   "_out.glb")
          .str);
  main.FinishAndSave(wr, std::string(ctx->workingFile.GetFolder()));
}
