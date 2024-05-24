#pragma once
#include "spike/type/vectors_simd.hpp"
#include <map>
#include <set>
#include <span>
#include <vector>

struct GLTF;

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
  Vector4A16 refRotation{0, 0, 0, 1};
  Vector4A16 refPosition;
  std::string_view positionCompression;
  std::string_view rotationCompression;
  std::string_view scaleCompression;
};

struct AnimEngine {
  std::map<size_t, AnimNode> nodes;
  std::set<size_t> usedIkNodes;
  uint32 numSamples;
};

using Hierarchy = std::set<size_t>;

void InheritScales(AnimEngine &eng, size_t animNode);
void LinkNodes(AnimEngine &eng, GLTF &main);

inline Vector4A16 Unpack(const SVector4 &i) {
  return Vector4A16(i.Convert<float>()) * (1.f / 0x7fff);
}

inline SVector4 Pack(Vector4A16 value) {
  value *= 0x7fff;
  value = Vector4A16(_mm_round_ps(value._data, _MM_ROUND_NEAREST));
  return value.Convert<int16>();
}

struct IkChainDescript;
using IkChainDescripts = std::span<IkChainDescript *>;
extern std::map<uint16, IkChainDescripts> IK_VERSION;

void SetupChains(AnimEngine &eng, IkChainDescripts iks);
