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

  const char *mainBuffer =
      main.buffer.data() + (self.vertexStart * self.buffer0Stride) +
      self.vertexStreamOffset + (self.indexValueOffset * self.buffer0Stride);
  auto curBuffer = mainBuffer;

  MODVertices vtArray;
  vtArray.numVertices = self.numVertices;

  auto newDesc = [&](uni::FormatDescr type, size_t size) {
    MODVertexDescriptor desc;
    desc.stride = stride;
    desc.offset = offset;
    desc.type = type;
    offset += size;
    desc.buffer = curBuffer + desc.offset;
    return desc;
  };

  auto makeBones = [&](size_t index) {
    MODVertexDescriptor boneIdx(
        newDesc({uni::FormatType::UINT, uni::DataType::R8G8B8A8}, 4));
    boneIdx.usage = uni::PrimitiveDescriptor::Usage_e::BoneIndices;
    boneIdx.index = index;
    vtArray.descs.storage.emplace_back(boneIdx);
  };

  auto makeWeights = [&](size_t index) {
    MODVertexDescriptor boneWt(
        newDesc({uni::FormatType::UNORM, uni::DataType::R8G8B8A8}, 4));
    boneWt.usage = uni::PrimitiveDescriptor::Usage_e::BoneWeights;
    boneWt.index = index;
    vtArray.descs.storage.emplace_back(boneWt);
  };

  auto makeNormals = [&](auto type) {
    MODVertexDescriptor norms(newDesc({uni::FormatType::NORM, type}, 4));

    if (laterFormat) {
      norms.type.compType = uni::DataType::R8G8B8A8;
      norms.type.outType = uni::FormatType::UNORM;
      norms.unpackType = uni::PrimitiveDescriptor::UnpackDataType_e::Madd;
      norms.unpackData.max = Vector4A16(-1.f);
      norms.unpackData.min = Vector4A16(2.f);
    }

    norms.usage = uni::PrimitiveDescriptor::Usage_e::Normal;
    vtArray.descs.storage.emplace_back(norms);

    if (!swap) {
      return;
    }

    auto normBuff = const_cast<char *>(norms.buffer);

    for (size_t i = 0; i < self.numVertices; i++) {
      FByteswapper(reinterpret_cast<uint32 &>(*normBuff));
      normBuff += self.buffer0Stride;
    }
  };

  auto makeTangents = [&] {
    MODVertexDescriptor tangs(
        newDesc({uni::FormatType::NORM, uni::DataType::R10G10B10A2}, 4));
    tangs.usage = uni::PrimitiveDescriptor::Usage_e::Tangent;
    vtArray.descs.storage.emplace_back(tangs);

    if (!swap) {
      return;
    }

    auto tangBuff = const_cast<char *>(tangs.buffer);

    for (size_t i = 0; i < self.numVertices; i++) {
      FByteswapper(reinterpret_cast<uint32 &>(*tangBuff));
      tangBuff += tangs.stride;
    }
  };

  auto makeUV = [&](size_t index) {
    MODVertexDescriptor uvset(
        newDesc({uni::FormatType::FLOAT, uni::DataType::R16G16}, 4));
    uvset.usage = uni::PrimitiveDescriptor::Usage_e::TextureCoordiante;
    uvset.index = index;
    vtArray.descs.storage.emplace_back(uvset);

    if (!swap) {
      return;
    }

    auto uvBuff = const_cast<char *>(uvset.buffer);

    for (size_t i = 0; i < self.numVertices; i++) {
      FByteswapper(reinterpret_cast<SVector2 &>(*uvBuff));
      uvBuff += uvset.stride;
    }
  };

  auto makeColor = [&](size_t index) {
    MODVertexDescriptor color(
        newDesc({uni::FormatType::UNORM, uni::DataType::R8G8B8A8}, 4));
    color.usage = uni::PrimitiveDescriptor::Usage_e::VertexColor;
    color.index = index;
    vtArray.descs.storage.emplace_back(color);

    if (!swap) {
      return;
    }

    auto colorBuff = const_cast<char *>(color.buffer);

    for (size_t i = 0; i < self.numVertices; i++) {
      auto &v = reinterpret_cast<uint32 &>(*colorBuff);
      FByteswapper(v);
      colorBuff += color.stride;
    }
  };

  if (useSkin) {
    MODVertexDescriptor pos(
        newDesc({laterFormat ? uni::FormatType::NORM : uni::FormatType::UNORM,
                 uni::DataType::R16G16B16A16},
                8));
    pos.usage = uni::PrimitiveDescriptor::Usage_e::Position;
    pos.unpackType = uni::PrimitiveDescriptor::UnpackDataType_e::Madd;
    pos.unpackData.max = main.bounds.bboxMin;
    pos.unpackData.min = main.bounds.bboxMax - main.bounds.bboxMin;

    vtArray.descs.storage.emplace_back(pos);

    if (swap) {
      auto posBuff = const_cast<char *>(pos.buffer);

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
    MODVertexDescriptor pos(
        newDesc({uni::FormatType::FLOAT, uni::DataType::R32G32B32}, 12));
    pos.usage = uni::PrimitiveDescriptor::Usage_e::Position;
    vtArray.descs.storage.emplace_back(pos);

    if (swap) {
      auto posBuff = const_cast<char *>(pos.buffer);

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
  retval.indexIndex = main.indices.Size();
  retval.vertexIndex = main.vertices.Size();

  MODIndices idArray;
  idArray.indexData = reinterpret_cast<const char *>(indexBuffer);
  idArray.numIndices = self.numIndices;

  for (size_t i = 0; i < self.numIndices; i++) {
    if (swap) {
      FByteswapper(indexBuffer[i]);
    }
    if (indexBuffer[i] != 0xffff) {
      indexBuffer[i] -= self.vertexStart;
    }
  }

  retval.indexType = uni::Primitive::IndexType_e::Strip;
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

MODVertices::MODVertices(std::initializer_list<MODVertexDescriptor> list) {
  descs.storage.insert(descs.storage.end(), list);
}

template <std::same_as<MODVertexDescriptor>... T>
MODVertices BuildVertices(T... items) {
  size_t offset = 0;
  static constexpr size_t fmtStrides[]{0,  128, 96, 64, 64, 48, 32, 32, 32,
                                       32, 32,  32, 24, 16, 16, 16, 16, 8};
  uint8 indices[0x10]{};

  auto NewDesc = [&](MODVertexDescriptor item) {
    item.offset = offset;
    item.index = indices[uint8(item.usage)]++;
    offset += fmtStrides[uint8(item.type.compType)] / 8;
    return item;
  };

  return MODVertices{NewDesc(items)...};
}

using F = uni::FormatType;
using D = uni::DataType;
using U = uni::PrimitiveDescriptor::Usage_e;
using V = MODVertexDescriptor;

static const MODVertexDescriptor VertexNormal = [] {
  V retVal{F::UNORM, D::R8G8B8A8, U::Normal};
  retVal.unpackType = uni::PrimitiveDescriptor::UnpackDataType_e::Madd;
  retVal.unpackData.min = Vector4A16{2};
  retVal.unpackData.max = Vector4A16{-1};
  return retVal;
}();

static const MODVertexDescriptor VertexTangent = [] {
  V retVal{F::UNORM, D::R8G8B8A8, U::Tangent};
  retVal.unpackType = uni::PrimitiveDescriptor::UnpackDataType_e::Madd;
  retVal.unpackData.min = Vector4A16{2};
  retVal.unpackData.max = Vector4A16{-1};
  return retVal;
}();

// clang-format off

static const MODVertexDescriptor TexCoord{F::FLOAT, D::R16G16, U::TextureCoordiante};

static const MODVertexDescriptor VertexQPosition{F::NORM, D::R16G16B16, U::Position};

static const MODVertexDescriptor VertexPosition{F::FLOAT, D::R32G32B32, U::Position};

static const MODVertexDescriptor VertexBoneIndices{F::UINT, D::R8G8B8A8, U::BoneIndices};

static const MODVertexDescriptor VertexColor{F::UNORM, D::R8G8B8A8, U::VertexColor};

std::map<uint32, MODVertices> formats{
    {
        0x64593023,
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      VertexBoneIndices,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneWeights},
                      V{F::UINT, D::R32, U::Undefined},
                      TexCoord,
                      V{F::UINT, D::R32, U::Undefined}),
    },
    {
        0x14d40020,
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      VertexBoneIndices,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneWeights}),
    },
    {
        0x207d6037,
        BuildVertices(VertexPosition,
                      VertexNormal,
                      TexCoord,
                      VertexColor),
    },
    {
        0x2f55c03d,
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      VertexBoneIndices,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneWeights},
                      V{F::UINT, D::R32, U::Undefined},
                      V{F::UINT, D::R32G32B32A32, U::Undefined},
                      V{F::UINT, D::R32G32B32A32, U::Undefined}),
    },
    {
        0x49b4f029,
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      VertexColor),
    },
    {
        0x5e7f202c,
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord),
    },
    {
        0x747d1031,
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      TexCoord),
    },
    {
        0x75c3e025,
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      V{F::UNORM, D::R8G8B8A8, U::BoneWeights},
                      VertexBoneIndices,
                      VertexBoneIndices,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneWeights},
                      VertexTangent,
                      TexCoord),
    },
    {
        0x926fd02e,
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      VertexColor),
    },
    {
        0xCBCF7027,
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      V{F::UNORM, D::R8G8B8A8, U::BoneWeights},
                      VertexBoneIndices,
                      VertexBoneIndices,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneWeights},
                      VertexTangent,
                      V{F::UINT, D::R32, U::Undefined},
                      TexCoord,
                      V{F::UINT, D::R32, U::Undefined}),
    },
    {
        0xbb424024,
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      V{F::UNORM, D::R8G8B8A8, U::BoneWeights},
                      VertexBoneIndices,
                      VertexBoneIndices,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneWeights},
                      VertexTangent),
    },
    {
        0xb392101f,
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      V{F::FLOAT, D::R16G16, U::BoneIndices},
                      TexCoord,
                      TexCoord,
                      V{F::UINT, D::R32G32, U::Undefined}),
    },
    {
        0xda55a021,
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      VertexBoneIndices,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneWeights},
                      TexCoord),
    },
    {
        0xd9e801d,
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneIndices},
                      TexCoord),
    },
    {
        0xc31f201c,
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneIndices}),
    },
    {
        0xd877801b,
        BuildVertices(VertexQPosition,
                      V{F::INT, D::R16, U::BoneIndices},
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      V{F::UINT, D::R32, U::Undefined},
                      TexCoord,
                      V{F::UINT, D::R32, U::Undefined}),
    },
    {
        0xcbf6c01a,
        BuildVertices(VertexQPosition,
                      V{F::UINT, D::R16, U::Undefined}, // bone?
                      VertexTangent,
                      VertexNormal,
                      TexCoord,
                      VertexColor),
    },
    {
        0x37a4e035,
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      TexCoord,
                      TexCoord),
    },
    {
        0x12553032,
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      TexCoord),
    },
    {
        0xafa6302d,
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord),
    },
    {
        0xd8297028,
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord),
    },
    {
        0x2082f03b,
        BuildVertices(VertexPosition,
                      VertexNormal,
                      TexCoord,
                      TexCoord,
                      TexCoord),
    },
    {
        0xc66fa03a,
        BuildVertices(VertexPosition,
                      VertexNormal,
                      TexCoord,
                      TexCoord),
    },
    {
        0xa7d7d036,
        BuildVertices(VertexPosition,
                      VertexNormal,
                      TexCoord),
    },
    {
        0xa14e003c,
        BuildVertices(VertexPosition,
                      VertexNormal,
                      TexCoord,
                      TexCoord,
                      VertexColor),
    },
    {
        0x9399c033,
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      VertexColor),
    },
    {
        0x4325a03e,
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      V{F::UINT, D::R32G32B32A32, U::Undefined},
                      V{F::UINT, D::R32G32B32A32, U::Undefined},
                      V{F::UINT, D::R32, U::Undefined}),
    },
    {
        0xb6681034,
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      VertexColor,
                      TexCoord),
    },
    {
        0xa14e003c,
        BuildVertices(VertexPosition,
                      VertexNormal,
                      TexCoord,
                      TexCoord,
                      VertexColor),
    },
    {
        0x63b6c02f,
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      TexCoord,
                      V{F::UINT, D::R32, U::Undefined}),
    },
    {
        0xa8fab018,
        BuildVertices(VertexQPosition,
                      V{F::UINT, D::R16, U::Undefined}, // bone?
                      VertexNormal,
                      VertexTangent,
                      TexCoord),
    },
};

// clang-format on

MODPrimitiveProxy MODMeshXD3::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXD3> &>(main_);
  MODPrimitiveProxy retval;
  uint8 visibleLOD = data0.Get<MODMeshXC5::VisibleLOD>();
  retval.lodIndex =
      convertLod(reinterpret_cast<es::Flags<uint8> &>(visibleLOD));
  retval.materialIndex = data0.Get<MODMeshXC5::MaterialIndex>();
  retval.indexType = uni::Primitive::IndexType_e::Triangle;
  retval.indexIndex = main.indices.Size();
  retval.vertexIndex = main.vertices.Size();
  retval.skinIndex = 0;
  retval.name = std::to_string(meshIndex) + ":" + std::to_string(data0.Get<MODMeshXC5::GroupIndex>());

  const char *mainBuffer = main.buffer.data() + (vertexStart * vertexStride) +
                           vertexStreamOffset +
                           (indexValueOffset * vertexStride);

  auto foundFormat = formats.find(vertexFormat);

  if (!es::IsEnd(formats, foundFormat)) {
    MODVertices tmpl = foundFormat->second;
    tmpl.numVertices = numVertices;
    for (auto &d : tmpl.descs.storage) {
      d.buffer = mainBuffer + d.offset;
      d.stride = vertexStride;

      if (d.usage == uni::PrimitiveDescriptor::Usage_e::BoneIndices &&
          skinBoneBegin) {
        d.unpackType = uni::PrimitiveDescriptor::UnpackDataType_e::Add;
        d.unpackData.min = Vector4A16(skinBoneBegin);
      }
    }
    main.vertices.storage.emplace_back(std::move(tmpl));
  } else {
    main.vertices.storage.emplace_back();
    // throw std::runtime_error("Unregistered vertex format: " +
    //         std::to_string(vertexFormat));
  }

  uint16 *indexBuffer = reinterpret_cast<uint16 *>(
      &main.buffer[0] + main.vertexBufferSize + (indexStart * 2));

  MODIndices idArray;
  idArray.indexData = reinterpret_cast<const char *>(indexBuffer);
  idArray.numIndices = numIndices;

  for (size_t i = 0; i < numIndices; i++) {
    if (indexBuffer[i] != 0xffff) {
      indexBuffer[i] -= vertexStart;
    }
  }

  main_.indices.storage.emplace_back(idArray);

  return retval;
}

std::string MODPrimitiveProxy::Name() const { return name; }
size_t MODPrimitiveProxy::SkinIndex() const { return skinIndex; }
int64 MODPrimitiveProxy::LODIndex() const { return lodIndex; }
size_t MODPrimitiveProxy::MaterialIndex() const { return materialIndex; }
uni::Primitive::IndexType_e MODPrimitiveProxy::IndexType() const {
  return indexType;
}
size_t MODPrimitiveProxy::VertexArrayIndex(size_t) const { return vertexIndex; }
size_t MODPrimitiveProxy::IndexArrayIndex() const { return indexIndex; }
size_t MODPrimitiveProxy::NumVertexArrays() const { return 1; }

MOD::MOD() {}
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
