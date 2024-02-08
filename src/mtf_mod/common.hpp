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
#include "spike/uni/list_vector.hpp"
#include "spike/uni/model.hpp"
#include "spike/uni/skeleton.hpp"

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
  es::Matrix44 localTransform;
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

struct MODMetaDataV2 : MODMetaDataV1 {
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

struct MODBoneProxy : uni::Bone {
  const revil::MODImpl &main;
  revil::BoneIndex index;
  MODBone data;

  MODBoneProxy(const revil::MODImpl &main_, size_t index_, const MODBone &data_)
      : main(main_), index(index_, data_.index), data(data_) {}
  uni::TransformType TMType() const override;
  void GetTM(es::Matrix44 &out) const override;
  const Bone *Parent() const override;
  size_t Index() const override;
  std::string Name() const override;
  operator uni::Element<const uni::Bone>() const {
    return uni::Element<const uni::Bone>{this, false};
  }
};

struct MODPrimitiveProxy : uni::Primitive {
  IndexType_e indexType;
  size_t skinIndex = 0;
  size_t lodIndex = 0;
  size_t materialIndex = 0;
  std::string name;
  size_t indexIndex;
  size_t vertexIndex;

  IndexType_e IndexType() const override;
  std::string Name() const override;
  size_t SkinIndex() const override;
  int64 LODIndex() const override;
  size_t MaterialIndex() const override;
  size_t VertexArrayIndex(size_t id) const override;
  size_t IndexArrayIndex() const override;
  size_t NumVertexArrays() const override;

  operator uni::Element<const uni::Primitive>() const {
    return uni::Element<const uni::Primitive>{this, false};
  }
};

struct MODIndices : uni::IndexArray {
  const char *indexData;
  size_t numIndices;
  const char *RawIndexBuffer() const override { return indexData; }
  size_t IndexSize() const override { return 2; }
  size_t NumIndices() const override { return numIndices; }

  operator uni::Element<const uni::IndexArray>() const {
    return uni::Element<const uni::IndexArray>{this, false};
  }
};

struct MODVertexDescriptor : uni::PrimitiveDescriptor {
  char *buffer;
  size_t stride;
  size_t offset;
  size_t index = 0;
  Usage_e usage;
  uni::FormatDescr type;
  uni::BBOX unpackData;
  UnpackDataType_e unpackType = UnpackDataType_e::None;
  uint32 typeSize = 0;
  bool packed = false;

  MODVertexDescriptor() = default;
  MODVertexDescriptor(uni::FormatType fmtType, uni::DataType dtType,
                      Usage_e usage_)
      : usage{usage_}, type{fmtType, dtType} {}

  const char *RawBuffer() const { return buffer; }
  size_t Stride() const { return stride; }
  size_t Offset() const { return offset; }
  size_t Index() const { return index; }
  Usage_e Usage() const { return usage; }
  uni::FormatDescr Type() const { return type; }
  uni::BBOX UnpackData() const { return unpackData; }
  UnpackDataType_e UnpackDataType() const { return unpackType; }

  operator uni::Element<const uni::PrimitiveDescriptor>() const {
    return uni::Element<const uni::PrimitiveDescriptor>{this, false};
  }
};

struct MODVertices : uni::VertexArray {
  uni::VectorList<uni::PrimitiveDescriptor, MODVertexDescriptor> descs;
  uint32 numVertices = 0;
  uint32 vertexStride = 0;

  MODVertices(MODVertices &&) = default;
  MODVertices(const MODVertices &) = default;
  MODVertices() = default;
  MODVertices(std::initializer_list<MODVertexDescriptor> list);

  uni::PrimitiveDescriptorsConst Descriptors() const override {
    return {&descs, false};
  }
  size_t NumVertices() const override { return numVertices; }

  operator uni::Element<const uni::VertexArray>() const {
    return uni::Element<const uni::VertexArray>{this, false};
  }
};

class revil::MODImpl : public uni::Skeleton, public uni::Model {
public:
  using ptr = std::unique_ptr<MODImpl>;
  uni::VectorList<uni::Bone, MODBoneProxy> boneData;
  uni::VectorList<uni::IndexArray, MODIndices> indices;
  uni::VectorList<uni::VertexArray, MODVertices> vertices;

  std::string buffer;
  std::vector<es::Matrix44> refPoses;
  std::vector<es::Matrix44> transforms;
  std::vector<MODEnvelope> envelopes;
  std::vector<MODGroup> groups;
  size_t vertexBufferSize;
  size_t indexBufferSize;
  MODBounds bounds;

  virtual ~MODImpl() = default;
  std::string Name() const override;
  uni::SkeletonBonesConst Bones() const override;
  virtual void Reflect(bool) = 0;
  uni::IndexArraysConst Indices() const override { return {&indices, false}; }
  uni::VertexArraysConst Vertices() const override {
    return {&vertices, false};
  }
};

struct MODSkinProxy : uni::Skin {
  size_t numRemaps;
  const uint8 *remaps = nullptr;
  const es::Matrix44 *poses;

  size_t NumNodes() const override;
  uni::TransformType TMType() const override;
  void GetTM(es::Matrix44 &out, size_t index) const override;
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

  size_t Version() const override;
  std::string Name() const override;
  std::string TypeName() const override;
  uni::MetadataConst Metadata() const override;

  void Write(BinWritterRef) const;
  void Read(BinReaderRef_e);

  operator uni::Element<const uni::Material>() const {
    return uni::Element<const uni::Material>{this, false};
  }
};

template <class traits> struct MODInner : revil::MODImpl {
  uni::VectorList<uni::Primitive, MODPrimitiveProxy> primitives;
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
