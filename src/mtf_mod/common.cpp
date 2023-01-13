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
#include <set>
#include <span>

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

  char *mainBuffer =
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

    if (!swap || laterFormat) {
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

  main.indices.storage.emplace_back(idArray);
  main.vertices.storage.emplace_back(std::move(vtArray));

  retval.indexType = uni::Primitive::IndexType_e::Strip;
  retval.name = std::to_string(self.unk);
  return retval;
};

MODPrimitiveProxyV1 MODMeshX70::ReflectBE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsX70> &>(main_);
  return makeV1(*this, main, true, false);
}

MODPrimitiveProxyV1 MODMeshX99::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsX99LE> &>(main_);
  auto retval = makeV1(*this, main, false, true);
  retval.skinIndex = skinInfo.boneRemapIndex;
  return retval;
}

MODPrimitiveProxyV1 MODMeshX99::ReflectBE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsX99BE> &>(main_);
  auto retval = makeV1(*this, main, true, true);
  retval.skinIndex = skinInfo.boneRemapIndex;
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

static const MODVertexDescriptor TexCoordPhone = [] {
  V retVal{F::UNORM, D::R16G16, U::TextureCoordiante};
  retVal.unpackType = uni::PrimitiveDescriptor::UnpackDataType_e::Mul;
  retVal.unpackData.min = Vector4A16{64};
  return retVal;
}();

// clang-format off

static const MODVertexDescriptor TexCoord{F::FLOAT, D::R16G16, U::TextureCoordiante};

static const MODVertexDescriptor VertexQPosition{F::NORM, D::R16G16B16, U::Position};

static const MODVertexDescriptor VertexPosition{F::FLOAT, D::R32G32B32, U::Position};

static const MODVertexDescriptor VertexBoneIndices{F::UINT, D::R8G8B8A8, U::BoneIndices};

static const MODVertexDescriptor VertexColor{F::UNORM, D::R8G8B8A8, U::VertexColor};

static const MODVertexDescriptor VertexNormalSigned{F::NORM, D::R8G8B8A8, U::Normal};

static const MODVertexDescriptor VertexTangentSigned{F::NORM, D::R8G8B8A8, U::Tangent};

std::map<uint32, MODVertices> formats{
    {
        0x64593023, // P3s_W1s_N4c_T4c_B4c_U2h_W2h_unk_U2h_unk
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
        0x14d40020, // P3s_W1s_N4c_T4c_B4c_U2h_W2h
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      VertexBoneIndices,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneWeights}),
    },
    {
        0x14d40019, // P3s_W1s_N4c_T4c_B4c_U2h_W2h
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      VertexBoneIndices,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneWeights}),
    },
    {
        0x14d40021, // P3s_W1s_N4c_T4c_B4c_U2h_W2h
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormalSigned,
                      VertexTangentSigned,
                      VertexBoneIndices,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneWeights}),
    },
    {
        0xbde5301a, // P3s_W1s_N4c_T4c_B4c_U2h_W2h
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      VertexBoneIndices,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneWeights}),
    },
    {
        0xd86ca01c, // P3s_W1s_N4c_T4c_B4c_U2h_W2h_VC4c
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      VertexBoneIndices,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneWeights},
                      VertexColor),
    },
    {
        0x77d87022, // P3s_W1s_N4c_T4c_B4c_U2h_W2h_VC4c
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      VertexBoneIndices,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneWeights},
                      VertexColor),
    },
    {
        0x77d87023, // P3s_W1s_N4c_T4c_B4c_U2h_W2h_VC4c
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormalSigned,
                      VertexTangentSigned,
                      VertexBoneIndices,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneWeights},
                      VertexColor),
    },
    {
        0x207d6037, // P3f_N4c_U2h_VC4c
        BuildVertices(VertexPosition,
                      VertexNormal,
                      TexCoord,
                      VertexColor),
    },
    {
        0x207d6030, // P3f_N4c_U2h_VC4c
        BuildVertices(VertexPosition,
                      VertexNormal,
                      TexCoord,
                      VertexColor),
    },
    {
        0x207d6038, // P3f_N4c_U2h_VC4c
        BuildVertices(VertexPosition,
                      VertexNormalSigned,
                      TexCoord,
                      VertexColor),
    },
    {
        0x2f55c03d, // P3s_W1s_N4c_T4c_B4c_U2h_W2h_unk36c
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
        0x49b4f029, // P3f_N4c_T4c_U2h_VC4c
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      VertexColor),
    },
    {
        0x49b4f022, // P3f_N4c_T4c_U2h_VC4c
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      VertexColor),
    },
    {
        0x49b4f02a, // P3f_N4c_T4c_U2h_VC4c
        BuildVertices(VertexPosition,
                      VertexNormalSigned,
                      VertexTangentSigned,
                      TexCoord,
                      VertexColor),
    },
    {
        0x5e7f202c, // P3f_N4c_T4c_U2h_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord),
    },
    {
        0x5e7f202d, // P3f_N4c_T4c_U2h_U2h
        BuildVertices(VertexPosition,
                      VertexNormalSigned,
                      VertexTangentSigned,
                      TexCoord,
                      TexCoord),
    },
    {
        0x747d1031, // P3f_N4c_T4c_U2h_U2h_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      TexCoord),
    },
    {
        0x75c3e025, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c_U2h
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
        0x926fd02e, // P3f_N4c_T4c_U2h_U2h_VC4c
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      VertexColor),
    },
    {
        0xb86de02a, // P3f_N4c_T4c_U2h_U2h_VC4c
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      VertexColor),
    },
    {
        0xCBCF7027, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c_unk1i_U2h_unk1i
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
        0xbb424024, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c
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
        0x1273701e, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c
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
        0xbb42401d, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c
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
        0xbb424025, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormalSigned,
                      V{F::UNORM, D::R8G8B8A8, U::BoneWeights},
                      VertexBoneIndices,
                      VertexBoneIndices,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneWeights},
                      VertexTangentSigned),
    },
    {
        0xb392101f, // P3s_W1s_N4c_T4c_B2h_U2h_U2h_unk2i
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
        0xda55a021, // P3s_W1s_N4c_T4c_B4c_U2h_W2s_U2h
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
        0xd9e801d, // P3s_W1s_N4c_T4c_U2h_B2h_U2h
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneIndices},
                      TexCoord),
    },
    {
        0xc31f201c, // P3s_W1s_N4c_T4c_U2h_B2h
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneIndices}),
    },
    {
        0x6a2e1016, // P3s_W1s_N4c_T4c_U2h_B2h
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneIndices}),
    },
    {
        0xc31f2014, // P3s_W1s_N4c_T4c_U2h_B2h
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneIndices}),
    },
    {
        0xc31f201d, // P3s_W1s_N4c_T4c_U2h_B2h
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormalSigned,
                      VertexTangentSigned,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneIndices}),
    },
    {
        0xa013501e, // P3s_W1s_N4c_T4c_U2h_B2h_VC4c
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneIndices},
                      VertexColor),
    },
    {
        0xa013501f, // P3s_W1s_N4c_T4c_U2h_B2h_VC4c
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormalSigned,
                      VertexTangentSigned,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneIndices},
                      VertexColor),
    },
    {
        0xd877801b, // P3s_B1s_N4c_T4c_U2h_unk1i_U2h_unk1i
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
        0xcbf6c01a, // P3s_unk1s_T4c_N4c_U2h_VC4c
        BuildVertices(VertexQPosition,
                      V{F::UINT, D::R16, U::Undefined}, // bone?
                      VertexTangent,
                      VertexNormal,
                      TexCoord,
                      VertexColor),
    },
    {
        0xcbf6c01b, // P3s_unk1s_T4c_N4c_U2h_VC4c
        BuildVertices(VertexQPosition,
                      V{F::UINT, D::R16, U::Undefined}, // bone?
                      VertexTangentSigned,
                      VertexNormalSigned,
                      TexCoord,
                      VertexColor),
    },
    {
        0x37a4e035, // P3s_N4c_T4c_U2h_U2h_U2h_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      TexCoord,
                      TexCoord),
    },
    {
        0x12553032, // P3s_N4c_T4c_U2h_U2h_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      TexCoord),
    },
    {
        0xafa6302d, // P3f_N4c_T4c_U2h_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord),
    },
    {
        0xd8297028, // P3f_N4c_T4c_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord),
    },
    {
        0xd8297021, // P3f_N4c_T4c_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord),
    },
    {
        0xd8297029, // P3f_N4c_T4c_U2h
        BuildVertices(VertexPosition,
                      VertexNormalSigned,
                      VertexTangentSigned,
                      TexCoord),
    },
    {
        0x2082f03b, // P3f_N4c_U2h_U2h_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      TexCoord,
                      TexCoord,
                      TexCoord),
    },
    {
        0xc66fa03a, // P3f_N4c_U2h_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      TexCoord,
                      TexCoord),
    },
    {
        0xd1a47038, // P3f_N4c_U2h_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      TexCoord,
                      TexCoord),
    },
    {
        0xa7d7d036, // P3f_N4c_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      TexCoord),
    },
    {
        0xa7d7d02f, // P3f_N4c_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      TexCoord),
    },
    {
        0xa7d7d037, // P3f_N4c_U2h
        BuildVertices(VertexPosition,
                      VertexNormalSigned,
                      TexCoord),
    },
    {
        0xa14e003c, // P3f_N4c_U2h_U2h_VC4c
        BuildVertices(VertexPosition,
                      VertexNormal,
                      TexCoord,
                      TexCoord,
                      VertexColor),
    },
    {
        0x9399c033, // P3f_N4c_T4c_U2h_U2h_VC4c
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      VertexColor),
    },
    {
        0x4325a03e, // P3f_N4c_T4c_U2h_U2h_unk3h_unk1s_unki7
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
        0xb6681034, // P3f_N4c_T4c_U2h_U2h_VC4c_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      VertexColor,
                      TexCoord),
    },
    {
        0x63b6c02f, // P3f_N4c_T4c_U2h_U2h_U2h_unk1i
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      TexCoord,
                      V{F::UINT, D::R32, U::Undefined}),
    },
    {
        0xa8fab018, // P3f_1s_N4c_T4c_U2h
        BuildVertices(VertexQPosition,
                      V{F::INT, D::R16, U::BoneIndices},
                      VertexNormal,
                      VertexTangent,
                      TexCoord),
    },
    {
        0xa8fab010, // P3f_1s_N4c_T4c_U2h
        BuildVertices(VertexQPosition,
                      V{F::INT, D::R16, U::BoneIndices},
                      VertexNormal,
                      VertexTangent,
                      TexCoord),
    },
    {
        0xa8fab019, // P3f_1s_N4c_T4c_U2h
        BuildVertices(VertexQPosition,
                      V{F::INT, D::R16, U::BoneIndices},
                      VertexNormalSigned,
                      VertexTangentSigned,
                      TexCoord),
    },
    {
        0x1cb8011, // P3s_B1s_N4c_T4c_U2h
        BuildVertices(VertexQPosition,
                      V{F::INT, D::R16, U::BoneIndices},
                      VertexNormal,
                      VertexTangent,
                      TexCoord),
    },
    {
        0x1cb8011, // P3s_B1s_N4c_T4c_U2h_U2h
        BuildVertices(VertexQPosition,
                      V{F::INT, D::R16, U::BoneIndices},
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord),
    },
    {
        0xd84e3026, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c_VC4c
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      V{F::UNORM, D::R8G8B8A8, U::BoneWeights},
                      VertexBoneIndices,
                      VertexBoneIndices,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneWeights},
                      VertexTangent,
                      VertexColor),
    },
    {
        0xd84e3027, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c_VC4c
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormalSigned,
                      V{F::UNORM, D::R8G8B8A8, U::BoneWeights},
                      VertexBoneIndices,
                      VertexBoneIndices,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneWeights},
                      VertexTangentSigned,
                      VertexColor),
    },
    {
        0xa8fab009, // P3s_unk1s_N4c_B4c_U2s_T4c
        BuildVertices(VertexQPosition,
                      V{F::UINT, D::R16, U::Undefined},
                      VertexNormalSigned,
                      VertexBoneIndices,
                      TexCoordPhone,
                      VertexTangentSigned),
    },
    {
        0xAE62600B, // P3s_unk1s_N4c_T4c_B4c_W4c_U2h
        BuildVertices(VertexQPosition,
                      V{F::UINT, D::R16, U::Undefined},
                      VertexNormalSigned,
                      VertexTangentSigned,
                      VertexBoneIndices,
                      V{F::UNORM, D::R8G8B8A8, U::BoneWeights},
                      TexCoordPhone),
    },
};

// clang-format on

static const std::set<uint32> edgeModels{
    0xdb7da014,
};

static const auto makeV2 = [](auto &self, revil::MODImpl &main, bool swap,
                              auto &&fd) {
  MODPrimitiveProxy retval;
  uint8 visibleLOD = self.data0.template Get<MODMeshXC5::VisibleLOD>();
  retval.lodIndex =
      convertLod(reinterpret_cast<es::Flags<uint8> &>(visibleLOD));
  retval.materialIndex = self.data0.template Get<MODMeshXC5::MaterialIndex>();
  retval.indexType = uni::Primitive::IndexType_e::Strip;
  retval.indexIndex = main.indices.Size();
  retval.vertexIndex = main.vertices.Size();
  retval.name = std::to_string(self.meshIndex) + ":" +
                std::to_string(self.data0.template Get<MODMeshXC5::GroupID>());
  const size_t vertexStride =
      self.data1.template Get<MODMeshXC5::VertexBufferStride>();

  char *mainBuffer = main.buffer.data() + (self.vertexStart * vertexStride) +
                     self.vertexStreamOffset +
                     (self.indexValueOffset * vertexStride);

  auto foundFormat = formats.find(self.vertexFormat);

  if (!es::IsEnd(formats, foundFormat)) {
    MODVertices tmpl = foundFormat->second;
    tmpl.numVertices = self.numVertices;
    for (auto &d : tmpl.descs.storage) {
      d.buffer = mainBuffer + d.offset;
      d.stride = vertexStride;
      fd(d);
    }
    main.vertices.storage.emplace_back(std::move(tmpl));
  } else {
    main.vertices.storage.emplace_back();
    // throw std::runtime_error("Unregistered vertex format: " +
    //         std::to_string(vertexFormat));
  }

  uint16 *indexBuffer = reinterpret_cast<uint16 *>(
      &main.buffer[0] + main.vertexBufferSize + (self.indexStart * 2));

  MODIndices idArray;
  idArray.indexData = reinterpret_cast<const char *>(indexBuffer);
  idArray.numIndices = self.numIndices;

  if (swap) {
    for (size_t i = 0; i < self.numIndices; i++) {
      if (indexBuffer[i] != 0xffff) {
        FByteswapper(indexBuffer[i]);
        indexBuffer[i] -= self.vertexStart;
      }
    }
  } else {
    for (size_t i = 0; i < self.numIndices; i++) {
      if (indexBuffer[i] != 0xffff) {
        indexBuffer[i] -= self.vertexStart;
      }
    }
  }

  main.indices.storage.emplace_back(idArray);

  return retval;
};

static const auto swapBuffers = [](MODVertexDescriptor &d, size_t numVertices) {
  char *curBuffer = d.buffer;

  switch (d.type.compType) {
  case uni::DataType::R16: {
    for (size_t v = 0; v < numVertices; v++, curBuffer += d.stride) {
      FByteswapper(*reinterpret_cast<uint16 *>(curBuffer));
    }
    break;
  }

  case uni::DataType::R16G16: {
    for (size_t v = 0; v < numVertices; v++, curBuffer += d.stride) {
      FByteswapper(*reinterpret_cast<USVector2 *>(curBuffer));
    }
    break;
  }

  case uni::DataType::R16G16B16: {
    for (size_t v = 0; v < numVertices; v++, curBuffer += d.stride) {
      FByteswapper(*reinterpret_cast<USVector *>(curBuffer));
    }
    break;
  }

  case uni::DataType::R32G32B32: {
    for (size_t v = 0; v < numVertices; v++, curBuffer += d.stride) {
      FByteswapper(*reinterpret_cast<Vector *>(curBuffer));
    }
    break;
  }

  default:
    break;
  }
};

MODPrimitiveProxy MODMeshXD2::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXD3> &>(main_);
  return makeV2(*this, main, false, [&](MODVertexDescriptor &d) {
    if (d.usage == uni::PrimitiveDescriptor::Usage_e::BoneIndices &&
        skinBoneBegin) {
      d.unpackType = uni::PrimitiveDescriptor::UnpackDataType_e::Add;
      d.unpackData.min = Vector4A16(skinBoneBegin);
    }
  });
}

MODPrimitiveProxy MODMeshXD2::ReflectBE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXD2> &>(main_);
  return makeV2(*this, main, true, [&](MODVertexDescriptor &d) {
    if (d.usage == uni::PrimitiveDescriptor::Usage_e::BoneIndices &&
        skinBoneBegin < main.bones.size()) {
      d.unpackType = uni::PrimitiveDescriptor::UnpackDataType_e::Add;
      d.unpackData.min = Vector4A16(skinBoneBegin);
    }

    swapBuffers(d, numVertices);
  });
}

MODPrimitiveProxy MODMeshXD3PS4::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXD2> &>(main_);
  return makeV2(*this, main, false, [&](MODVertexDescriptor &d) {
    if (d.usage == uni::PrimitiveDescriptor::Usage_e::BoneIndices &&
        skinBoneBegin) {
      d.unpackType = uni::PrimitiveDescriptor::UnpackDataType_e::Add;
      d.unpackData.min = Vector4A16(skinBoneBegin);
    } else if (d.usage == uni::PrimitiveDescriptor::Usage_e::Normal) {
      d.type = VertexNormalSigned.type;
      d.unpackType = uni::PrimitiveDescriptor::UnpackDataType_e::None;
    } else if (d.usage == uni::PrimitiveDescriptor::Usage_e::Tangent) {
      d.type = VertexTangentSigned.type;
      d.unpackType = uni::PrimitiveDescriptor::UnpackDataType_e::None;
    }
  });
}

MODPrimitiveProxy MODMeshXC5::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXC5> &>(main_);
  return makeV2(*this, main, false, [&](MODVertexDescriptor &) {});
}

MODPrimitiveProxy MODMeshX06::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXD3> &>(main_);
  auto retval = makeV2(*this, main, false, [&](MODVertexDescriptor &) {});
  retval.skinIndex = skinBoneBegin;

  /*auto idxArray = main.Indices()->At(retval.indexIndex);
  std::span<const uint16> indices(
      reinterpret_cast<const uint16 *>(idxArray->RawIndexBuffer()), numIndices);

  bool removeTS = true;

  for (auto d : vtArray.descs) {
    switch (d->Usage()) {
    case uni::PrimitiveDescriptor::Usage_e::Normal: {
      uni::FormatCodec::fvec sampled;
      d->Codec().Sample(sampled, d->RawBuffer(), numVertices, d->Stride());
      for (auto i : indices) {
        if (i == 0xffff) {
          continue;
        }

        auto s = sampled.at(i);
        if (s.Length() > 0.5f) {
          removeTS = false;
          //break;
        } else {
          printf("%i ", i);
        }
      }

      break;
    }

    default:
      break;
    }
  }

  if (removeTS) {
    std::remove_if(vtArray.descs.storage.begin(), vtArray.descs.storage.end(),
                   [](MODVertexDescriptor &d) {
                     return d.usage == MODVertexDescriptor::Usage_e::Normal;
                   });
    std::remove_if(vtArray.descs.storage.begin(), vtArray.descs.storage.end(),
                   [](MODVertexDescriptor &d) {
                     return d.usage == MODVertexDescriptor::Usage_e::Tangent;
                   });
  }*/

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
