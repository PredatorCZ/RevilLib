/*  Revil Format Library
    Copyright(C) 2017-2026 Lukas Cone

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

#pragma once
#include "revil/platform.hpp"
#include "spike/gltf_attribute.hpp"
#include "spike/io/bincore_fwd.hpp"
#include "spike/type/flags.hpp"
#include "spike/util/pugi_fwd.hpp"
#include "types.hpp"
#include <memory>
#include <span>
#include <string>
#include <vector>

class Reflector;

namespace revil {

class MODImpl;

enum MODVersion : uint16 {
  X05 = 0x05,
  X06 = 0x06,
  X70 = 0x70,   // DR X360
  X170 = 0x170, // DR PC
  X99 = 0x99,   // MTF1 generic
  X19C = 0x19c, // RE5 PC
  XD2 = 0xD2,
  XD3 = 0xD3, // RE6 PC
  XD4 = 0xD4,
  XD6 = 0xD6,
  XFF2C = 0xFF2C, // RE6 PS3
  XC3 = 0xC3,     // LP2 PS3
  XC5 = 0xC5,     // LP2 PC
  XE5 = 0xE5,
  XE6 = 0xE6,
  XE7 = 0xE7,
  X21 = 0x21,
};

struct alignas(8) MODMaker {
  MODVersion version;
  bool swappedEndian = false;
  bool x64 = false;
  Platform platform = Platform::Auto;

  bool operator<(const MODMaker &i0) const;
};

struct MODBone {
  uint16 index;
  uint16 parentIndex;
};

struct MODEnvelope {
  uint32 boneIndex;
  Vector4A16 boundingSphere;
  MtAABB aabb;
  MtOBB obb;
};

struct MODGroup {
  uint32 index;
  Vector4A16 boundingSphere;
};

struct MODMetaData {
  uint32 middleDistance;
  uint32 lowDistance;
  uint32 lightGroup;
  uint8 boundaryJoint;
};

struct MODPrimitive {
  enum class Flags : uint16 {
    Lod1,
    Lod2,
    Lod3,
    Visible,
    Shape,
    Bridge,
    Connective,
    Sort,
    BinormalFlip,
    TriStrips,
  };

  es::Flags<Flags> flags;
  uint8 alphaType;
  uint16 drawMode = 0;
  uint16 skinIndex = 0;
  uint16 materialIndex = 0;
  uint32 indexIndex;
  uint32 vertexIndex;
  uint16 groupId;
  uint16 meshId;
  uint32 layout = 0;
};

using MODSkinJoints = std::span<const uint8>;

struct MODVertexSpan {
  char *buffer;
  uint32 numVertices;
  uint32 stride;
  std::vector<Attribute> attrs;
  std::vector<std::unique_ptr<AttributeCodec>> codecs;
};

using MODIndexSpan = std::span<uint16>;

struct MODMaterial {
  virtual std::string GetName() const = 0;
  virtual void ToXML(pugi::xml_node node) const = 0;
};

class ES_EXPORT MOD {
public:
  MOD();
  MOD(MODMaker make);
  MOD(MOD &&);
  ~MOD();
  /* Castable into:
  uni::Element<const uni::Skeleton>
  */
  template <class C> C As() const;
  void Load(const std::string &fileName);
  void Load(BinReaderRef_e rd);
  void Save(BinWritterRef wr);
  void ToXML(pugi::xml_node node) const;

  std::span<const MODVertexSpan> Vertices() const;
  std::span<const MODIndexSpan> Indices() const;
  std::span<const MODPrimitive> Primitives() const;
  std::span<const MODSkinJoints> SkinJoints() const;
  std::span<const MODMaterial *> Materials() const;
  std::span<const es::Matrix44> InverseBinds() const;
  std::span<const es::Matrix44> Transforms() const;
  std::span<const MODBone> Bones() const;
  std::span<const MODGroup> Groups() const;
  std::span<const MODEnvelope> Envelopes() const;
  const MODMetaData &Metadata() const;
  const std::vector<std::string> &Textures() const;

private:
  std::unique_ptr<MODImpl> pi;
};

} // namespace revil
