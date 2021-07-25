/*  Revil Format Library
    Copyright(C) 2017-2021 Lukas Cone

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
#include "datas/matrix44.hpp"
#include "datas/reflector.hpp"
#include "revil/mod.hpp"
#include "uni/list_vector.hpp"
#include "uni/model.hpp"
#include "uni/skeleton.hpp"

namespace revil {
class MODImpl;
}

struct MODBone {
  uint16 index;
  uint16 parentIndex;
  uint16 mirrorIndex;
  float furthestVertexDistance;
  float parentDistance;
  Vector absolutePosition;
};

struct MODBounds {
  Vector4A16 boundingSphere;
  Vector4A16 bboxMin;
  Vector4A16 bboxMax;
};

struct MODEnvelope {
  uint32 boneIndex;
  MODBounds bounds;
  esMatrix44 localTransform;
  Vector4A16 absolutePosition;
};

struct MODGroup {
  uint32 index;
  Vector4A16 boundingSphere;
};

struct MODMetaDataV1 {
  uint32 middleDistance;
  uint32 lowDistance;
  uint32 lightGroup;
  uint8 boundaryJoint; //??
};

template <size_t numRemaps> struct MODSkinRemap {
  uint32 count;
  uint8 bones[numRemaps];
};

template <size_t size> struct MODPath {
  char path[size];
  void SwapEndian() {}
};

struct MODBoneProxy : uni::Bone {
  const revil::MODImpl &main;
  revil::BoneIndex index;
  MODBone data;

  MODBoneProxy(const revil::MODImpl &main_, size_t index_, const MODBone &data_)
      : main(main_), index(index_, data_.index), data(data_) {}
  uni::TransformType TMType() const override;
  void GetTM(esMatrix44 &out) const override;
  const Bone *Parent() const override;
  size_t Index() const override;
  std::string Name() const override;
  operator uni::Element<const uni::Bone>() const {
    return uni::Element<const uni::Bone>{this, false};
  }
};

struct MODPrimitiveDescriptorProxy : uni::PrimitiveDescriptor {
  const char *rawBuffer;
  size_t stride;
  size_t offset;
  size_t index = 0;
  Usage_e usage;
  uni::FormatDescr desc;
  uni::BBOX unpackData;
  UnpackDataType_e unpackType = UnpackDataType_e::None;

  const char *RawBuffer() const override;
  size_t Stride() const override;
  size_t Offset() const override;
  size_t Index() const override;
  Usage_e Usage() const override;
  uni::FormatDescr Type() const override;
  uni::BBOX UnpackData() const override;
  UnpackDataType_e UnpackDataType() const override;

  operator uni::Element<const uni::PrimitiveDescriptor>() const {
    return uni::Element<const uni::PrimitiveDescriptor>{this, false};
  }
};

struct MODPrimitiveProxy : uni::Primitive {
  uni::VectorList<uni::PrimitiveDescriptor, MODPrimitiveDescriptorProxy> descs;
  const char *mainBuffer;
  const char *indexBuffer;
  IndexType_e indexType;
  size_t numVertices;
  size_t numIndices;
  size_t skinIndex = 0;
  size_t lodIndex = 0;
  size_t materialIndex = 0;
  std::string name;

  const char *RawIndexBuffer() const override;
  const char *RawVertexBuffer(size_t id) const override;
  uni::PrimitiveDescriptorsConst Descriptors() const override;
  IndexType_e IndexType() const override;
  size_t IndexSize() const override;
  size_t NumVertices() const override;
  size_t NumVertexBuffers() const override;
  size_t NumIndices() const override;
  std::string Name() const override;
  size_t SkinIndex() const override;
  size_t LODIndex() const override;
  size_t MaterialIndex() const override;

  operator uni::Element<const uni::Primitive>() const {
    return uni::Element<const uni::Primitive>{this, false};
  }
};

struct MODPrimitiveProxyV1 : MODPrimitiveProxy {
  const char *additionalBuffer;

  const char *RawVertexBuffer(size_t id) const override;
  size_t NumVertexBuffers() const override;
};

class revil::MODImpl : public uni::Skeleton, public uni::Model {
public:
  using ptr = std::unique_ptr<MODImpl>;
  uni::VectorList<uni::Bone, MODBoneProxy> boneData;

  std::string buffer;
  std::vector<esMatrix44> refPoses;
  std::vector<esMatrix44> transforms;
  std::vector<MODEnvelope> envelopes;
  std::vector<MODGroup> groups;
  size_t vertexBufferSize;
  size_t indexBufferSize;
  MODBounds bounds;

  virtual ~MODImpl() = default;
  std::string Name() const override;
  uni::SkeletonBonesConst Bones() const override;
  virtual void Reflect(bool) = 0;
};

struct MODSkinProxy : uni::Skin {
  size_t numRemaps;
  const uint8 *remaps = nullptr;
  const esMatrix44 *poses;

  size_t NumNodes() const override;
  uni::TransformType TMType() const override;
  void GetTM(esMatrix44 &out, size_t index) const override;
  size_t NodeIndex(size_t index) const override;
  operator uni::Element<const uni::Skin>() const {
    return uni::Element<const uni::Skin>{this, false};
  }
};

class MODPathProxy {
public:
  std::string path;
  operator uni::Element<const std::string>() const {
    return uni::Element<const std::string>{&path, false};
  }
};

template <class material_type> class MODMaterialProxy : public uni::Material {
public:
  material_type main;
  ReflectorWrap<material_type> asMetadata{main};

  virtual size_t Version() const override;
  virtual std::string Name() const override;
  virtual std::string TypeName() const override;
  virtual uni::MetadataConst Metadata() const override;

  void Write(BinWritterRef) const;
  void Read(BinReaderRef);

  operator uni::Element<const uni::Material>() const {
    return uni::Element<const uni::Material>{this, false};
  }
};

template <class traits> struct MODInner : revil::MODImpl {
  uni::VectorList<uni::Primitive, typename traits::primitive> primitives;
  uni::VectorList<uni::Skin, MODSkinProxy> skins;
  uni::VectorList<std::string, MODPathProxy> paths;
  uni::VectorList<uni::Material, MODMaterialProxy<typename traits::material>>
      materials;
  std::vector<typename traits::bone> bones;
  std::vector<MODSkinRemap<traits::numSkinRemaps>> skinRemaps;
  uint8 remaps[traits::numRemaps];
  std::vector<typename traits::mesh> meshes;
  size_t unkBufferSize;
  typename traits::metadata metadata;

  void Reflect(bool) override;
  uni::PrimitivesConst Primitives() const override;
  uni::SkinsConst Skins() const override;
  uni::ResourcesConst Resources() const override;
  uni::MaterialsConst Materials() const override;
};
