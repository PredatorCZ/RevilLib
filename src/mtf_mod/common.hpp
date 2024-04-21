/*  Revil Format Library
    Copyright(C) 2017-2023 Lukas Cone

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
#include "revil/mod.hpp"
#include "spike/reflect/reflector.hpp"
#include "spike/type/matrix44.hpp"

namespace revil {
class MODImpl;
}

struct MODMetaDataV2 : revil::MODMetaData {
  uint32 numEnvelopes;
};

template <size_t numRemaps> struct MODSkinRemap {
  uint32 count;
  uint8 bones[numRemaps];
};

template <size_t size> struct MODPath {
  char path[size];
  void NoSwap();
};

class revil::MODImpl {
public:
  using ptr = std::unique_ptr<MODImpl>;

  std::string vertexBuffer;
  std::vector<uint16> indexBuffer;
  std::vector<es::Matrix44> refPoses;
  std::vector<es::Matrix44> transforms;
  std::vector<MODEnvelope> envelopes;
  std::vector<MODGroup> groups;
  MODBounds bounds;
  MODMetaData simpleMetadata;
  std::vector<MODPrimitive> primitives;
  std::vector<MODSkinJoints> skins;
  std::vector<std::string> paths;
  std::vector<MODVertexSpan> vertices;
  std::vector<MODIndexSpan> indices;
  std::vector<MODMaterial> materialRefs;
  std::vector<MODBone> simpleBones;

  virtual ~MODImpl() = default;
  MODImpl(const MODImpl &) = delete;
  MODImpl() = default;
  MODImpl(MODImpl &&) = default;

  virtual void Reflect(bool) = 0;
  virtual const MODMetaData &Metadata() const = 0;
};

template <class material_type> class MODMaterialProxy {
public:
  material_type main;
  ReflectorWrap<material_type> asMetadata{main};

  void Write(BinWritterRef) const;
  void Read(BinReaderRef_e);
};

template <class traits> struct MODInner : revil::MODImpl {
  std::vector<MODMaterialProxy<typename traits::material>> materials;
  std::vector<typename traits::bone> bones;
  std::vector<MODSkinRemap<traits::numSkinRemaps>> skinRemaps;

  uint8 remaps[traits::numRemaps];
  std::vector<typename traits::mesh> meshes;
  size_t unkBufferSize;
  typename traits::metadata metadata;

  void Reflect(bool) override;
  const revil::MODMetaData &Metadata() const override { return metadata; }
};
