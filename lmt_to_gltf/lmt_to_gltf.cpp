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

#include "datas/binreader_stream.hpp"
#include "datas/binwritter_stream.hpp"
#include "datas/fileinfo.hpp"
#include "datas/matrix44.hpp"
#include "gltf.hpp"
#include "project.h"
#include "re_common.hpp"
#include "revil/lmt.hpp"
#include <set>

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

struct LMT2GLTF : ReflectorBase<LMT2GLTF> {
  std::string modelSource;
} settings;

REFLECT(CLASS(LMT2GLTF),
        MEMBERNAME(modelSource, "model-source", "s",
                   ReflDesc{
                       "Set path to model gltf as a base for animtions."}));

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = LMT2GLTF_DESC " v" LMT2GLTF_VERSION ", " LMT2GLTF_COPYRIGHT
                            "Lukas Cone",
    .settings = reinterpret_cast<ReflectorFriend *>(&settings),
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

private:
  int32 aniStream = -1;
};

struct AnimNode {
  std::vector<SVector4> rotations;
  std::vector<Vector4A16> positions;
  std::vector<Vector4A16> scales;
  std::vector<Vector4A16> globalPositions;
  std::vector<SVector4> globalRotations;
  std::vector<uint32> children;
  int32 glNodeIndex = -1;
  int32 glScaleNodeIndex = -1;
  int32 parentAnimNode = -1;
  float magnitude = 0;
  uint8 boneType = 0;
  Vector4A16 refRotation;
  Vector4A16 refPosition;
  std::string_view positionCompression;
  std::string_view rotationCompression;
  std::string_view scaleCompression;
};

struct AnimEngine {
  std::map<size_t, AnimNode> nodes;
  uint32 numSamples;
};

void WalkTree(AnimEngine &eng, GLTF &main, gltf::Node &glNode, int32 parent) {
  auto found = glNode.name.find(':');
  AnimNode *aNode = nullptr;
  int32 animNodeId = -1;

  if (glNode.name.ends_with("_s")) {
    return;
  }

  if (found != std::string::npos) {
    animNodeId = std::atol(glNode.name.data() + found + 1);
  }

  if (!eng.nodes.contains(animNodeId)) {
    AnimNode anNode;
    anNode.glNodeIndex = std::distance(main.nodes.data(), &glNode);
    aNode = &eng.nodes.emplace(animNodeId, anNode).first->second;
  }

  aNode = &eng.nodes.at(animNodeId);

  memcpy((void *)&aNode->refPosition, glNode.translation.data(), 12);
  memcpy((void *)&aNode->refRotation, glNode.rotation.data(), 16);
  aNode->magnitude = aNode->refPosition.Length();
  aNode->parentAnimNode = parent;

  if (parent != animNodeId) {
    eng.nodes.at(parent).children.push_back(animNodeId);
  }

  parent = animNodeId;
  std::string sName = glNode.name + "_s";

  for (auto childId : glNode.children) {
    if (main.nodes.at(childId).name == sName) {
      aNode->glScaleNodeIndex = childId;
      break;
    }

    WalkTree(eng, main, main.nodes.at(childId), parent);
  }
}

void LinkNodes(AnimEngine &eng, GLTF &main) {
  for (auto &node : main.nodes) {
    if (node.name == "reference") {
      WalkTree(eng, main, node, -1);
      return;
    }
  }
}

void InheritScales(AnimEngine &eng, size_t animNode) {
  auto &aNode = eng.nodes.at(animNode);

  if (aNode.parentAnimNode >= 0) {
    auto &aParentNode = eng.nodes.at(aNode.parentAnimNode);

    if (!aParentNode.scales.empty()) {
      const size_t numFrames = aParentNode.scales.size();

      if (aNode.scales.empty()) {
        aNode.scales = aParentNode.scales;
      } else {
        for (size_t f = 0; f < numFrames; f++) {
          aNode.scales.at(f) *= aParentNode.scales.at(f);
        }
      }

      if (aNode.positions.empty()) {
        aNode.positions.insert(aNode.positions.begin(), numFrames,
                               aNode.refPosition);
      }

      for (size_t f = 0; f < numFrames; f++) {
        aNode.positions.at(f) *= aParentNode.scales.at(f);
      }
    }
  }

  for (auto &child : aNode.children) {
    InheritScales(eng, child);
  }
}

struct IkChain {
  uint32 base;
  int32 controlBase = -1;
};

using Hierarchy = std::set<size_t>;

void MarkHierarchy(AnimEngine &eng, Hierarchy &marks, size_t endNode) {
  if (auto parentIndex = eng.nodes.at(endNode).parentAnimNode;
      parentIndex >= 0) {
    marks.emplace(parentIndex);
    MarkHierarchy(eng, marks, parentIndex);
  }
}

#include "glm/gtx/quaternion.hpp"

Vector4A16 TransformPoint(Vector4A16 q, Vector4A16 point) {
  glm::quat q_(q.w, q.x, q.y, q.z);
  glm::vec3 p_(point.x, point.y, point.z);
  auto res = q_ * p_;

  return Vector4A16(res.x, res.y, res.z, 0);
}

Vector4A16 Multiply(Vector4A16 parent, Vector4A16 child) {
  glm::quat p_(parent.w, parent.x, parent.y, parent.z);
  glm::quat c_(child.w, child.x, child.y, child.z);
  auto res = c_ * p_;

  return Vector4A16(res.x, res.y, res.z, res.w);
}

/*
Vector4A16 TransformPoint(Vector4A16 q, Vector4A16 point) {
  Vector4A16 qvec = q * Vector4A16(1, 1, 1, 0);
  Vector4A16 uv = qvec.Cross(point);
  Vector4A16 uuv = qvec.Cross(uv);

  return point + ((uv * q.x) + uuv) * 2;
}

Vector4A16 Multiply(Vector4A16 parent, Vector4A16 child) {
  Vector4A16 tier0 = Vector4A16(child.x, child.y, child.z, child.w) * parent.w;
  Vector4A16 tier1 = Vector4A16(child.w, child.w, child.w, -child.x) *
                     Vector4A16(parent.x, parent.y, parent.z, parent.x);
  Vector4A16 tier2 = Vector4A16(child.z, child.x, child.y, -child.y) *
                     Vector4A16(parent.y, parent.z, parent.x, parent.y);
  Vector4A16 tier3 = Vector4A16(child.y, child.z, child.x, child.z) *
                     Vector4A16(parent.z, parent.x, parent.y, parent.z);

  return tier0 + tier1 + tier2 - tier3;
}*/

Vector4A16 Unpack(const SVector4 &i) {
  return Vector4A16(i.Convert<float>()) * (1.f / 0x7fff);
}

SVector4 Pack(Vector4A16 value) {
  value *= 0x7fff;
  value = Vector4A16(_mm_round_ps(value._data, _MM_ROUND_NEAREST));
  return value.Convert<int16>();
}

void MakeGlobalFrames(AnimEngine &eng, Hierarchy &marks, size_t nodeId) {
  auto &aNode = eng.nodes.at(nodeId);
  auto &parentNode = eng.nodes.at(aNode.parentAnimNode);

  aNode.globalPositions = parentNode.globalPositions;
  aNode.globalRotations.resize(eng.numSamples);

  if (aNode.positions.empty()) {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalPositions.at(s) += TransformPoint(
          Unpack(parentNode.globalRotations.at(s)), aNode.refPosition);
    }
  } else {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalPositions.at(s) += TransformPoint(
          Unpack(parentNode.globalRotations.at(s)), aNode.positions.at(s));
    }
  }

  if (aNode.rotations.empty()) {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalRotations.at(s) = Pack(Multiply(
          Unpack(parentNode.globalRotations.at(s)), aNode.refRotation));
    }
  } else {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalRotations.at(s) =
          Pack(Multiply(Unpack(parentNode.globalRotations.at(s)),
                        Unpack(aNode.rotations.at(s))));
    }
  }

  for (auto child : aNode.children) {
    if (marks.contains(child)) {
      MakeGlobalFrames(eng, marks, child);
    }
  }
}

void MakeGlobalFrames(AnimEngine &eng, Hierarchy &marks) {
  auto &refNode = eng.nodes.at(size_t(-1));

  if (refNode.rotations.empty()) {
    refNode.rotations.insert(refNode.rotations.begin(), eng.numSamples,
                             SVector4(0, 0, 0, 0x7fff));
  }

  if (refNode.positions.empty()) {
    refNode.positions.resize(eng.numSamples);
  }

  refNode.globalPositions = refNode.positions;
  refNode.globalRotations = refNode.rotations;

  for (auto child : refNode.children) {
    if (marks.contains(child)) {
      MakeGlobalFrames(eng, marks, child);
    }
  }
}

void MakeGlobalIkControl(AnimEngine &eng, IkChain chain) {
  auto &aNode = eng.nodes.at(chain.base + 2);
  auto &parentNode = eng.nodes.at(chain.controlBase);

  aNode.globalPositions = parentNode.globalPositions;
  aNode.globalRotations.resize(eng.numSamples);

  if (aNode.positions.empty()) {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalPositions.at(s) += TransformPoint(
          Unpack(parentNode.globalRotations.at(s)), aNode.refPosition);
    }
  } else {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalPositions.at(s) += TransformPoint(
          Unpack(parentNode.globalRotations.at(s)), aNode.positions.at(s));
    }
  }

  if (aNode.rotations.empty()) {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalRotations.at(s) = Pack(Multiply(
          Unpack(parentNode.globalRotations.at(s)), aNode.refRotation));
    }
  } else {
    for (size_t s = 0; s < eng.numSamples; s++) {
      aNode.globalRotations.at(s) =
          Pack(Multiply(Unpack(parentNode.globalRotations.at(s)),
                        Unpack(aNode.rotations.at(s))));
    }
  }
}

void RebakeChain(AnimEngine &eng, size_t nodeId) {
  size_t endNode = nodeId + 2;
  auto &eNode = eng.nodes.at(endNode);
  auto &parentNode = eng.nodes.at(eNode.parentAnimNode);
  eNode.positions = std::move(eNode.globalPositions);

  if (eNode.globalRotations.empty()) {
    eNode.rotations.insert(eNode.rotations.begin(), eng.numSamples,
                           SVector4(0, 0, 0, 0x7fff));
  } else {
    eNode.rotations = std::move(eNode.globalRotations);
  }

  for (size_t s = 0; s < eng.numSamples; s++) {
    eNode.positions.at(s) -= parentNode.globalPositions.at(s);
    eNode.positions.at(s) =
        TransformPoint(Unpack(parentNode.globalRotations.at(s)).QConjugate(),
                       eNode.positions.at(s));

    eNode.rotations.at(s) =
        Pack(Multiply(Unpack(parentNode.globalRotations.at(s)).QConjugate(),
                      Unpack(eNode.rotations.at(s))));
  }
}

void SetupChains(AnimEngine &eng) {
  std::vector<IkChain> chains;
  Hierarchy marks;

  for (auto &[nodeIndex, node] : eng.nodes) {
    if (node.boneType && node.boneType != 2) {
      IkChain nChain;
      nChain.base = nodeIndex;

      /*if (node.boneType > 20) {
        nChain.controlBase = 9;
        MarkHierarchy(eng, marks, 9);
        marks.emplace(9);
      }*/

      chains.emplace_back(nChain);

      node.positions.clear(); // Unknown purpose, not positions
      MarkHierarchy(eng, marks, nodeIndex + 2);
    }
  }

  if (chains.empty()) {
    return;
  }

  MakeGlobalFrames(eng, marks);

  for (auto &chain : chains) {
    MakeGlobalIkControl(eng, chain);
    RebakeChain(eng, chain.base);
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
              std::span<float> times, uint32 loopFrame, ReportType &report) {

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
      keyAccess.min.push_back(0);
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

    main.animations.emplace_back(std::move(animation));
  };

  if (loopFrame) {
    uint32 maxKeys = std::max(loopFrame, uint32(times.size()) - loopFrame);
    std::span<float> timesSpan(times.data(), maxKeys);

    size_t keyIndexStart = [&] {
      auto &stream = main.AnimStream();
      auto [keyAccess, keyIndex] = main.NewAccessor(stream, 4);
      keyAccess.type = gltf::Accessor::Type::Scalar;
      keyAccess.componentType = gltf::Accessor::ComponentType::Float;
      keyAccess.min.push_back(0);
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
      keyAccess.min.push_back(0);
      keyAccess.max.push_back(times.back());
      keyAccess.count = times.size();
      stream.wr.WriteContainer(times);

      return keyIndex;
    }();

    Write(loopFrame, times.size(), keyIndex, animName);
  }
}

#include "datas/master_printer.hpp"

// Ak09, Ak15(Dongo) and Most Vs doesn't have marked LegIKTarget
enum LPTypes {
  None,
  Unk1,
  LegIKTarget,
  Ak00RightLegChain, // Vs03, Vs33
  Ak00LeftLegChain,  // Vs03, Vs33
  Unk5,
  Unk6,
  Ak09RightLegChain,     // Ak11, Ak13, Ak15, Vs06, Hm
  Ak09LeftLegChain,      // Ak0b front leg, Ak11, Ak13, Ak15, Vs06, Hm
  Ak0bRightBackLegChain, // Vs01, Vs03, Vs33, Vs34
  Ak0bLeftBackLegChain,  //  Vs01, Vs03, Vs33, Vs34
  Vs04RightLegChain,     // Vs05, Vs31, Vs32, Vs40
  Vs04LeftLegChain,      // Vs05, Vs31, Vs32, Vs40
  Vs00RightLegChain,     // Vs41
  Vs00LeftLegChain,      // Vs41
  Unk15,
  Unk16,
  Unk17,
  Unk18,
  Unk19,
  Unk20,
  Vs00RightArmChain, // Vs07, Vs41, Hm target parent ani_bone 9 neck
  Vs00LeftArmChain,  // Vs07, Vs41, Hm
};

void DoLmt(LMTGLTF &main, uni::MotionsConst motion, std::string name,
           ReportType &report) {
  int32 sampleRate = 60;
  const float sampleFrac = 1.f / sampleRate;

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
        printline("Missing bone: " << index);
        continue;
      }

      AnimNode &aNode = engine.nodes.at(index);
      auto tm = static_cast<const LMTTrack *>(t.get());
      aNode.boneType = tm->BoneType();

      if (aNode.boneType) {
        printline("Index: " << index << " type: " << int(aNode.boneType));
      }

      switch (t->TrackType()) {
      case uni::MotionTrack::Position:
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
        aNode.rotations.reserve(times.size());
        aNode.rotationCompression = tm->CompressionType();

        for (auto k : times) {
          Vector4A16 value;
          t->GetValue(value, k);
          aNode.rotations.emplace_back(Pack(value));
        }
        break;
      case uni::MotionTrack::Scale:
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
    SetupChains(engine);

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
    BinWritterRef wrj(ctx->NewFile(
        std::string(ctx->workingFile.GetFullPathNoExt()) + "_report.json").str);

    wrj.BaseStream() << report;
  }
#endif

  BinWritterRef wr(ctx->NewFile(
      std::string(ctx->workingFile.GetFullPathNoExt()) + "_out.glb").str);
  main.FinishAndSave(wr, std::string(ctx->workingFile.GetFolder()));
}
