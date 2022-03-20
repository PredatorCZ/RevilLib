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

#include "traits.hpp"

using namespace revil;

uni::TransformType MODBoneProxy::TMType() const { return uni::TMTYPE_MATRIX; }

size_t MODBoneProxy::Index() const { return index; }

std::string MODBoneProxy::Name() const { return ""; }

void MODBoneProxy::GetTM(es::Matrix44 &out) const {
  out = main.refPoses[index.id];
}

const uni::Bone *MODBoneProxy::Parent() const {
  return data.parentIndex == 0xff || data.parentIndex == 0xffff
             ? nullptr
             : &main.boneData.storage[data.parentIndex];
}

std::string MODImpl::Name() const { return ""; }

uni::SkeletonBonesConst MODImpl::Bones() const { return {&boneData, false}; }

const char *MODPrimitiveDescriptorProxy::RawBuffer() const { return rawBuffer; }
size_t MODPrimitiveDescriptorProxy::Stride() const { return stride; }
size_t MODPrimitiveDescriptorProxy::Offset() const { return offset; }
size_t MODPrimitiveDescriptorProxy::Index() const { return index; }
uni::PrimitiveDescriptor::Usage_e MODPrimitiveDescriptorProxy::Usage() const {
  return usage;
}
uni::FormatDescr MODPrimitiveDescriptorProxy::Type() const { return desc; }
uni::BBOX MODPrimitiveDescriptorProxy::UnpackData() const { return unpackData; }
uni::PrimitiveDescriptor::UnpackDataType_e
MODPrimitiveDescriptorProxy::UnpackDataType() const {
  return unpackType;
}

size_t MODSkinProxy::NumNodes() const { return numRemaps; }
uni::TransformType MODSkinProxy::TMType() const { return uni::TMTYPE_MATRIX; }
void MODSkinProxy::GetTM(es::Matrix44 &out, size_t index) const {
  if (remaps) {
    out = poses[remaps[index]];
  } else {
    out = poses[index];
  }
}
size_t MODSkinProxy::NodeIndex(size_t index) const {
  return remaps ? remaps[index] : index;
}

inline size_t convertLod(es::Flags<uint8> vis) {
  LODIndex index;
  index.lod1 = vis[0];
  index.lod2 = vis[1];
  index.lod3 = vis[2];
  return index;
}

static const auto makeV1 = [](auto &self, auto &main, bool swap,
                              bool laterFormat) {
  auto &mat = main.materials.storage[self.materialIndex].main;
  using material_type = typename std::remove_reference<decltype(mat)>::type;
  auto useSkin = mat.vshData.template Get<typename material_type::SkinType>();
  bool skin8 = laterFormat ? useSkin == 4 : useSkin == 2;

  size_t offset = 0;
  size_t stride = self.buffer0Stride;
  MODPrimitiveProxyV1 retval;
  retval.lodIndex = convertLod(self.visibleLOD);
  retval.materialIndex = self.materialIndex;
  // retval.name = "group_" + std::to_string(self.unk);

  retval.mainBuffer =
      main.buffer.data() + (self.vertexStart * self.buffer0Stride) +
      self.vertexStreamOffset + (self.indexValueOffset * self.buffer0Stride);
  auto curBuffer = retval.mainBuffer;

  auto newDesc = [&](uni::FormatDescr type, size_t size) {
    MODPrimitiveDescriptorProxy desc;
    desc.stride = stride;
    desc.offset = offset;
    desc.desc = type;
    offset += size;
    desc.rawBuffer = curBuffer + desc.offset;
    return desc;
  };

  auto makeBones = [&](size_t index) {
    MODPrimitiveDescriptorProxy boneIdx(
        newDesc({uni::FormatType::UINT, uni::DataType::R8G8B8A8}, 4));
    boneIdx.usage = uni::PrimitiveDescriptor::Usage_e::BoneIndices;
    boneIdx.index = index;
    retval.descs.storage.emplace_back(boneIdx);
  };

  auto makeWeights = [&](size_t index) {
    MODPrimitiveDescriptorProxy boneWt(
        newDesc({uni::FormatType::UNORM, uni::DataType::R8G8B8A8}, 4));
    boneWt.usage = uni::PrimitiveDescriptor::Usage_e::BoneWeights;
    boneWt.index = index;
    retval.descs.storage.emplace_back(boneWt);
  };

  auto makeNormals = [&](auto type) {
    MODPrimitiveDescriptorProxy norms(
        newDesc({uni::FormatType::NORM, type}, 4));

    if (laterFormat) {
      norms.desc.compType = uni::DataType::R8G8B8A8;
      norms.desc.outType = uni::FormatType::UNORM;
      norms.unpackType = uni::PrimitiveDescriptor::UnpackDataType_e::Madd;
      norms.unpackData.max = Vector4A16(-1.f);
      norms.unpackData.min = Vector4A16(2.f);
    }

    norms.usage = uni::PrimitiveDescriptor::Usage_e::Normal;
    retval.descs.storage.emplace_back(norms);

    if (!swap) {
      return;
    }

    auto normBuff = const_cast<char *>(norms.rawBuffer);

    for (size_t i = 0; i < self.numVertices; i++) {
      FByteswapper(reinterpret_cast<uint32 &>(*normBuff));
      normBuff += self.buffer0Stride;
    }
  };

  auto makeTangents = [&] {
    MODPrimitiveDescriptorProxy tangs(
        newDesc({uni::FormatType::NORM, uni::DataType::R10G10B10A2}, 4));
    tangs.usage = uni::PrimitiveDescriptor::Usage_e::Tangent;
    retval.descs.storage.emplace_back(tangs);

    if (!swap) {
      return;
    }

    auto tangBuff = const_cast<char *>(tangs.rawBuffer);

    for (size_t i = 0; i < self.numVertices; i++) {
      FByteswapper(reinterpret_cast<uint32 &>(*tangBuff));
      tangBuff += tangs.stride;
    }
  };

  auto makeUV = [&](size_t index) {
    MODPrimitiveDescriptorProxy uvset(
        newDesc({uni::FormatType::FLOAT, uni::DataType::R16G16}, 4));
    uvset.usage = uni::PrimitiveDescriptor::Usage_e::TextureCoordiante;
    uvset.index = index;
    retval.descs.storage.emplace_back(uvset);

    if (!swap) {
      return;
    }

    auto uvBuff = const_cast<char *>(uvset.rawBuffer);

    for (size_t i = 0; i < self.numVertices; i++) {
      FByteswapper(reinterpret_cast<SVector2 &>(*uvBuff));
      uvBuff += uvset.stride;
    }
  };

  auto makeColor = [&](size_t index) {
    MODPrimitiveDescriptorProxy color(
        newDesc({uni::FormatType::UNORM, uni::DataType::R8G8B8A8}, 4));
    color.usage = uni::PrimitiveDescriptor::Usage_e::VertexColor;
    color.index = index;
    retval.descs.storage.emplace_back(color);

    if (!swap) {
      return;
    }

    auto colorBuff = const_cast<char *>(color.rawBuffer);

    for (size_t i = 0; i < self.numVertices; i++) {
      auto &v = reinterpret_cast<uint32 &>(*colorBuff);
      FByteswapper(v);
      colorBuff += color.stride;
    }
  };

  if (useSkin) {
    MODPrimitiveDescriptorProxy pos(
        newDesc({laterFormat ? uni::FormatType::NORM : uni::FormatType::UNORM,
                 uni::DataType::R16G16B16A16},
                8));
    pos.usage = uni::PrimitiveDescriptor::Usage_e::Position;
    pos.unpackType = uni::PrimitiveDescriptor::UnpackDataType_e::Madd;
    pos.unpackData.max = main.bounds.bboxMin;
    pos.unpackData.min = main.bounds.bboxMax - main.bounds.bboxMin;

    retval.descs.storage.emplace_back(pos);

    if (swap) {
      auto posBuff = const_cast<char *>(pos.rawBuffer);

      for (size_t i = 0; i < self.numVertices; i++) {
        FByteswapper(reinterpret_cast<USVector4 &>(*posBuff));
        posBuff += self.buffer0Stride;
      }
    }

    if (skin8) {
      makeBones(0);
      makeBones(1);
      makeWeights(0);
      makeWeights(1);
      makeNormals(uni::DataType::R10G10B10A2);
      makeUV(0);
    } else {
      makeBones(0);
      makeWeights(0);
      makeNormals(uni::DataType::R10G10B10A2);
      makeTangents();
      makeUV(0);
      makeUV(1);
    }
  } else {
    MODPrimitiveDescriptorProxy pos(
        newDesc({uni::FormatType::FLOAT, uni::DataType::R32G32B32}, 12));
    pos.usage = uni::PrimitiveDescriptor::Usage_e::Position;
    retval.descs.storage.emplace_back(pos);

    if (swap) {
      auto posBuff = const_cast<char *>(pos.rawBuffer);

      for (size_t i = 0; i < self.numVertices; i++) {
        auto &vec = reinterpret_cast<Vector &>(*posBuff);
        FByteswapper(vec);
        posBuff += self.buffer0Stride;
      }
    }

    makeNormals(uni::DataType::R8G8B8A8);

    if (self.buffer1Stride != 8) {
      makeTangents();
      makeUV(0);
      makeUV(1);
      // makeColor(0); // null or swapped with tangent
    } else {
      offset += 4; // makeColor(0); // always null?
      makeUV(0);
      makeUV(1);
      makeTangents();
    }
  }

  if (skin8 && self.buffer1Stride != 8) {
    throw std::runtime_error("Expected secondary buffer of 8 bytes!");
  }

  if (!skin8 && useSkin && self.buffer1Stride) {
    throw std::runtime_error("Unexpected secondary buffer for skin!");
  }

  if (self.buffer1Stride) {
    retval.additionalBuffer = main.buffer.data() + main.vertexBufferSize;
    retval.additionalBuffer += self.vertexStream2Offset;
    offset = 0;
    stride = self.buffer1Stride;
    curBuffer = retval.additionalBuffer;

    if (skin8) {
      makeTangents();
      makeUV(1);
    } else {
      makeColor(0);

      if (offset < self.buffer1Stride) {
        makeColor(1);
      }
    }
  }

  uint16 *indexBuffer =
      reinterpret_cast<uint16 *>(&main.buffer[0] + main.vertexBufferSize +
                                 main.unkBufferSize + (self.indexStart * 2));
  retval.indexBuffer = reinterpret_cast<const char *>(indexBuffer);

  for (size_t i = 0; i < self.numIndices; i++) {
    if (swap) {
      FByteswapper(indexBuffer[i]);
    }
    if (indexBuffer[i] != 0xffff) {
      indexBuffer[i] -= self.vertexStart;
    }
  }

  retval.indexType = uni::Primitive::IndexType_e::Strip;
  retval.numIndices = self.numIndices;
  retval.numVertices = self.numVertices;
  return retval;
};

MODPrimitiveProxyV1 MODMeshX70::ReflectBE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsX70> &>(main_);
  return makeV1(*this, main, true, false);
}

MODPrimitiveProxyV1 MODMeshX99::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsX99LE> &>(main_);
  auto retval = makeV1(*this, main, false, true);
  retval.skinIndex = boneRemapIndex;
  return retval;
}

MODPrimitiveProxyV1 MODMeshX99::ReflectBE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsX99BE> &>(main_);
  auto retval = makeV1(*this, main, true, true);
  retval.skinIndex = boneRemapIndex;
  return retval;
}

const char *MODPrimitiveProxy::RawIndexBuffer() const { return indexBuffer; }
const char *MODPrimitiveProxy::RawVertexBuffer(size_t) const {
  return mainBuffer;
}
uni::PrimitiveDescriptorsConst MODPrimitiveProxy::Descriptors() const {
  return {&descs, false};
}
uni::Primitive::IndexType_e MODPrimitiveProxy::IndexType() const {
  return indexType;
}
size_t MODPrimitiveProxy::IndexSize() const { return 2; }
size_t MODPrimitiveProxy::NumVertices() const { return numVertices; }
size_t MODPrimitiveProxy::NumVertexBuffers() const { return 1; }
size_t MODPrimitiveProxy::NumIndices() const { return numIndices; }
std::string MODPrimitiveProxy::Name() const { return name; }
size_t MODPrimitiveProxy::SkinIndex() const { return skinIndex; }
size_t MODPrimitiveProxy::LODIndex() const { return lodIndex; }
size_t MODPrimitiveProxy::MaterialIndex() const { return materialIndex; }

const char *MODPrimitiveProxyV1::RawVertexBuffer(size_t id) const {
  if (id == 1 && additionalBuffer) {
    return additionalBuffer;
  }

  if (id) {
    throw std::out_of_range("RawVertexBuffer out of range.");
  }

  return mainBuffer;
}

size_t MODPrimitiveProxyV1::NumVertexBuffers() const {
  return additionalBuffer ? 2 : 1;
}

static bool registered = false;
void RegisterMaterials();

MOD::MOD() {
  if (registered) {
    return;
  }
  registered = true;
  RegisterMaterials();
}
MOD::MOD(MOD &&) = default;
MOD::~MOD() = default;

namespace revil {
template <>
ES_EXPORT uni::Element<const uni::Skeleton>
MOD::As<uni::Element<const uni::Skeleton>>() const {
  return {static_cast<const uni::Skeleton *>(this->pi.get()), false};
}

template <>
ES_EXPORT uni::Element<const uni::Model>
MOD::As<uni::Element<const uni::Model>>() const {
  return {static_cast<const uni::Model *>(this->pi.get()), false};
}
} // namespace revil
