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

MODVertices::MODVertices(std::initializer_list<MODVertexDescriptor> list) {
  descs.storage.insert(descs.storage.end(), list);

  for (auto &d : descs.storage) {
    vertexStride += d.typeSize;
  }
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
    item.typeSize = fmtStrides[uint8(item.type.compType)] / 8;
    offset += item.typeSize;
    return item;
  };

  return MODVertices{NewDesc(items)...};
}

void RebuildVertices(MODVertices &items) {
  size_t offset = 0;
  static constexpr size_t fmtStrides[]{0,  128, 96, 64, 64, 48, 32, 32, 32,
                                       32, 32,  32, 24, 16, 16, 16, 16, 8};
  uint8 indices[0x10]{};

  for (auto &item : items.descs.storage) {
    item.offset = offset;
    item.index = indices[uint8(item.usage)]++;
    item.typeSize = fmtStrides[uint8(item.type.compType)] / 8;
    offset += item.typeSize;
  }
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

static const MODVertexDescriptor VertexNormal3 = [] {
  V retVal{F::UNORM, D::R8G8B8, U::Normal};
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

static const MODVertexDescriptor VertexTangent3 = [] {
  V retVal{F::UNORM, D::R8G8B8, U::Tangent};
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

MODVertexDescriptor Packed(MODVertexDescriptor in) {
  in.packed = true;
  return in;
}

// clang-format off

static const MODVertexDescriptor TexCoord{F::FLOAT, D::R16G16, U::TextureCoordiante};

static const MODVertexDescriptor VertexQPosition{F::NORM, D::R16G16B16, U::Position};

static const MODVertexDescriptor VertexPosition{F::FLOAT, D::R32G32B32, U::Position};

static const MODVertexDescriptor VertexBoneIndices{F::UINT, D::R8G8B8A8, U::BoneIndices};

static const MODVertexDescriptor VertexColor{F::UNORM, D::R8G8B8A8, U::VertexColor};

static const MODVertexDescriptor VertexNormalSigned{F::NORM, D::R8G8B8A8, U::Normal};

static const MODVertexDescriptor VertexTangentSigned{F::NORM, D::R8G8B8A8, U::Tangent};

// clang-format on

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

  case uni::DataType::R16G16B16A16: {
    for (size_t v = 0; v < numVertices; v++, curBuffer += d.stride) {
      FByteswapper(*reinterpret_cast<USVector4 *>(curBuffer));
    }
    break;
  }

  case uni::DataType::R32:
  case uni::DataType::R10G10B10A2: {
    for (size_t v = 0; v < numVertices; v++, curBuffer += d.stride) {
      FByteswapper(*reinterpret_cast<uint32 *>(curBuffer));
    }
    break;
  }

  case uni::DataType::R8G8B8A8:
    if (d.packed) {
      for (size_t v = 0; v < numVertices; v++, curBuffer += d.stride) {
        FByteswapper(*reinterpret_cast<uint32 *>(curBuffer));
      }
    }
    break;

  case uni::DataType::R32G32: {
    for (size_t v = 0; v < numVertices; v++, curBuffer += d.stride) {
      FByteswapper(*reinterpret_cast<Vector2 *>(curBuffer));
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

static const auto makeVertices0X70 = [](uint8 useSkin, bool v1stride4,
                                        auto &main) {
  MODVertices vtArray;
  const bool skin8 = useSkin == 2;

  if (useSkin) {
    MODVertexDescriptor pos{F::UNORM, D::R16G16B16A16, U::Position};
    pos.unpackType = uni::PrimitiveDescriptor::UnpackDataType_e::Madd;
    pos.unpackData.max = main.bounds.bboxMin;
    pos.unpackData.min = main.bounds.bboxMax - main.bounds.bboxMin;
    vtArray.descs.storage.emplace_back(pos);

    if (skin8) {
      vtArray.descs.storage.emplace_back(VertexBoneIndices);
      vtArray.descs.storage.emplace_back(VertexBoneIndices);
      vtArray.descs.storage.emplace_back(F::UNORM, D::R8G8B8A8, U::BoneWeights);
      vtArray.descs.storage.emplace_back(F::UNORM, D::R8G8B8A8, U::BoneWeights);
      vtArray.descs.storage.emplace_back(F::NORM, D::R10G10B10A2, U::Normal);
      vtArray.descs.storage.emplace_back(TexCoord);
    } else {
      vtArray.descs.storage.emplace_back(VertexBoneIndices);
      vtArray.descs.storage.emplace_back(F::UNORM, D::R8G8B8A8, U::BoneWeights);
      vtArray.descs.storage.emplace_back(F::NORM, D::R10G10B10A2, U::Normal);
      vtArray.descs.storage.emplace_back(F::NORM, D::R10G10B10A2, U::Tangent);
      vtArray.descs.storage.emplace_back(TexCoord);
    }
  } else {
    vtArray.descs.storage.emplace_back(VertexPosition);
    vtArray.descs.storage.emplace_back(Packed(VertexNormalSigned));

    if (v1stride4) {
      vtArray.descs.storage.emplace_back(Packed(VertexTangentSigned));
      vtArray.descs.storage.emplace_back(TexCoord);
      vtArray.descs.storage.emplace_back(TexCoord);
      // null or swapped with tangent
      vtArray.descs.storage.emplace_back(F::UINT, D::R32, U::Undefined);
    } else {
      // always null?
      vtArray.descs.storage.emplace_back(F::UINT, D::R32, U::Undefined);
      vtArray.descs.storage.emplace_back(TexCoord);
      vtArray.descs.storage.emplace_back(TexCoord);
      vtArray.descs.storage.emplace_back(Packed(VertexTangentSigned));
    }
  }

  return vtArray;
};

static const auto makeVertices1X70 = [](uint8 useSkin, bool v1stride4) {
  MODVertices vtArray;
  const bool skin8 = useSkin == 2;

  if (skin8) {
    vtArray.descs.storage.emplace_back(F::NORM, D::R10G10B10A2, U::Tangent);
    vtArray.descs.storage.emplace_back(TexCoord);
  } else {
    vtArray.descs.storage.emplace_back(VertexColor);

    if (!v1stride4) {
      vtArray.descs.storage.emplace_back(VertexColor);
    }
  }
  return vtArray;
};

static const auto makeVertices0X99 = [](uint8 useSkin, bool v1stride4,
                                        auto &main) {
  MODVertices vtArray;
  const bool skin8 = useSkin == 4;

  if (useSkin) {
    MODVertexDescriptor pos{F::NORM, D::R16G16B16A16, U::Position};
    pos.unpackType = uni::PrimitiveDescriptor::UnpackDataType_e::Madd;
    pos.unpackData.max = main.bounds.bboxMin;
    pos.unpackData.min = main.bounds.bboxMax - main.bounds.bboxMin;
    vtArray.descs.storage.emplace_back(pos);

    if (skin8) {
      vtArray.descs.storage.emplace_back(VertexBoneIndices);
      vtArray.descs.storage.emplace_back(VertexBoneIndices);
      vtArray.descs.storage.emplace_back(F::UNORM, D::R8G8B8A8, U::BoneWeights);
      vtArray.descs.storage.emplace_back(F::UNORM, D::R8G8B8A8, U::BoneWeights);
      vtArray.descs.storage.emplace_back(VertexNormal);
      vtArray.descs.storage.emplace_back(TexCoord);
    } else {
      vtArray.descs.storage.emplace_back(VertexBoneIndices);
      vtArray.descs.storage.emplace_back(F::UNORM, D::R8G8B8A8, U::BoneWeights);
      vtArray.descs.storage.emplace_back(VertexNormal);
      vtArray.descs.storage.emplace_back(VertexTangent);
      vtArray.descs.storage.emplace_back(TexCoord);
    }
  } else {
    vtArray.descs.storage.emplace_back(VertexPosition);
    vtArray.descs.storage.emplace_back(VertexNormal);

    if (v1stride4) {
      vtArray.descs.storage.emplace_back(VertexTangent);
      vtArray.descs.storage.emplace_back(TexCoord);
      vtArray.descs.storage.emplace_back(TexCoord);
      // null or swapped with tangent
      vtArray.descs.storage.emplace_back(F::UINT, D::R32, U::Undefined);
    } else {
      // always null?
      vtArray.descs.storage.emplace_back(F::UINT, D::R32, U::Undefined);
      vtArray.descs.storage.emplace_back(TexCoord);
      vtArray.descs.storage.emplace_back(TexCoord);
      vtArray.descs.storage.emplace_back(VertexTangent);
    }
  }

  return vtArray;
};

static const auto makeVertices1X99 = [](uint8 useSkin, bool v1stride4) {
  MODVertices vtArray;
  const bool skin8 = useSkin == 4;

  if (skin8) {
    vtArray.descs.storage.emplace_back(VertexTangent);
    vtArray.descs.storage.emplace_back(TexCoord);
  } else {
    vtArray.descs.storage.emplace_back(VertexColor);

    if (!v1stride4) {
      vtArray.descs.storage.emplace_back(VertexColor);
    }
  }
  return vtArray;
};

static const auto makeVertices0X170 = [](uint8 useSkin, bool, auto &main) {
  MODVertices vtArray;
  const bool skin8 = useSkin == 2;

  if (useSkin) {
    MODVertexDescriptor pos{F::NORM, D::R16G16B16A16, U::Position};
    pos.unpackType = uni::PrimitiveDescriptor::UnpackDataType_e::Madd;
    pos.unpackData.max = main.bounds.bboxMin;
    pos.unpackData.min = main.bounds.bboxMax - main.bounds.bboxMin;
    vtArray.descs.storage.emplace_back(pos);

    if (skin8) {
      vtArray.descs.storage.emplace_back(VertexBoneIndices);
      vtArray.descs.storage.emplace_back(VertexBoneIndices);
      vtArray.descs.storage.emplace_back(F::UNORM, D::R8G8B8A8, U::BoneWeights);
      vtArray.descs.storage.emplace_back(F::UNORM, D::R8G8B8A8, U::BoneWeights);
      vtArray.descs.storage.emplace_back(VertexNormal);
      vtArray.descs.storage.emplace_back(TexCoord);
    } else {
      vtArray.descs.storage.emplace_back(VertexBoneIndices);
      vtArray.descs.storage.emplace_back(F::UNORM, D::R8G8B8A8, U::BoneWeights);
      vtArray.descs.storage.emplace_back(F::NORM, D::R16G16B16A16, U::Normal);
      vtArray.descs.storage.emplace_back(TexCoord);
    }
  } else {
    vtArray.descs.storage.emplace_back(VertexPosition);
    vtArray.descs.storage.emplace_back(F::NORM, D::R16G16B16A16, U::Normal);
    vtArray.descs.storage.emplace_back(F::FLOAT, D::R32G32,
                                       U::TextureCoordiante);
  }

  return vtArray;
};

static const auto makeVertices1X170 = [](uint8 useSkin, bool) {
  MODVertices vtArray;
  const bool skin8 = useSkin == 2;

  if (useSkin) {
    vtArray.descs.storage.emplace_back(F::NORM, D::R16G16B16A16, U::Tangent);
    vtArray.descs.storage.emplace_back(TexCoord);
  } else if (skin8) {
    vtArray.descs.storage.emplace_back(VertexTangent);
    vtArray.descs.storage.emplace_back(TexCoord);
  } else {
    vtArray.descs.storage.emplace_back(F::NORM, D::R16G16B16A16, U::Tangent);
    vtArray.descs.storage.emplace_back(F::FLOAT, D::R32G32,
                                       U::TextureCoordiante);
    vtArray.descs.storage.emplace_back(VertexColor);
    vtArray.descs.storage.emplace_back(VertexColor);
    vtArray.descs.storage.emplace_back(VertexColor);
  }
  return vtArray;
};

static const auto makeV1 = [](auto &self, auto &main, auto v0Maker,
                              auto v1Maker, auto &&fd) {
  auto &mat = main.materials.storage[self.materialIndex].main;
  using material_type = typename std::remove_reference<decltype(mat)>::type;
  auto useSkin = mat.vshData.template Get<typename material_type::SkinType>();

  MODPrimitiveProxy retval;
  retval.lodIndex = convertLod(self.visibleLOD);
  retval.materialIndex = self.materialIndex;
  // retval.name = "group_" + std::to_string(self.unk);
  retval.indexType = uni::Primitive::IndexType_e::Strip;
  retval.name = std::to_string(self.unk);
  retval.vertexIndex = main.vertices.Size();

  MODVertices vt0Array = v0Maker(useSkin, self.buffer1Stride != 8, main);
  vt0Array.numVertices = self.numVertices;
  RebuildVertices(vt0Array);

  char *mainBuffer =
      main.buffer.data() + (self.vertexStart * self.buffer0Stride) +
      self.vertexStreamOffset + (self.indexValueOffset * self.buffer0Stride);

  for (auto &d : vt0Array.descs.storage) {
    d.buffer = mainBuffer + d.offset;
    d.stride = self.buffer0Stride;
    fd(d);
  }

  main.vertices.storage.emplace_back(std::move(vt0Array));

  /*if (skin8 && self.buffer1Stride != 8) {
    throw std::runtime_error("Expected secondary buffer of 8 bytes!");
  }

  if (!skin8 && useSkin && self.buffer1Stride) {
    throw std::runtime_error("Unexpected secondary buffer for skin!");
  }*/

  if (self.buffer1Stride) {
    char *additionalBuffer =
        main.buffer.data() + main.vertexBufferSize + self.vertexStream2Offset;

    MODVertices vt1Array = v1Maker(useSkin, self.buffer1Stride != 8);
    vt1Array.numVertices = self.numVertices;
    RebuildVertices(vt1Array);

    for (auto &d : vt1Array.descs.storage) {
      d.buffer = additionalBuffer + d.offset;
      d.stride = self.buffer1Stride;
      fd(d);
    }

    main.vertices.storage.emplace_back(std::move(vt1Array));
  }

  uint16 *indexBuffer =
      reinterpret_cast<uint16 *>(&main.buffer[0] + main.vertexBufferSize +
                                 main.unkBufferSize + (self.indexStart * 2));
  retval.indexIndex = main.indices.Size();

  MODIndices idArray;
  idArray.indexData = reinterpret_cast<const char *>(indexBuffer);
  idArray.numIndices = self.numIndices;

  for (size_t i = 0; i < self.numIndices; i++) {
    if (indexBuffer[i] != 0xffff) {
      indexBuffer[i] -= self.vertexStart;
    }
  }

  main.indices.storage.emplace_back(idArray);

  return retval;
};

MODPrimitiveProxy MODMeshX70::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsX170> &>(main_);
  return makeV1(*this, main, makeVertices0X170, makeVertices1X170,
                [&](MODVertexDescriptor &) {});
}

MODPrimitiveProxy MODMeshX70::ReflectBE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsX70> &>(main_);
  return makeV1(*this, main, makeVertices0X70, makeVertices1X70,
                [&](MODVertexDescriptor &d) { swapBuffers(d, numVertices); });
}

MODPrimitiveProxy MODMeshX99::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsX99LE> &>(main_);
  auto retval = makeV1(*this, main, makeVertices0X99, makeVertices1X99,
                       [&](MODVertexDescriptor &) {});
  retval.skinIndex = skinInfo.boneRemapIndex;
  return retval;
}

MODPrimitiveProxy MODMeshX99::ReflectBE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsX99BE> &>(main_);
  auto retval =
      makeV1(*this, main, makeVertices0X99, makeVertices1X99,
             [&](MODVertexDescriptor &d) { swapBuffers(d, numVertices); });
  retval.skinIndex = skinInfo.boneRemapIndex;
  return retval;
}

// clang-format off

std::map<uint32, MODVertices> fallbackFormats{
    {
        // xc3
        0xc31f2014, // P3s_W1s_N3c_B1c_T3c_B1c_U2h
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal3,
                      V{F::UINT, D::R8, U::BoneIndices},
                      VertexTangent3,
                      V{F::UINT, D::R8, U::BoneIndices},
                      TexCoord),
    },
    {
        // xd2 be, this might be correct after all
        0xd8297028, // P3f_N4c_T4c_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      VertexColor),
    },
};

std::map<uint32, MODVertices> formats{
    {
        0x64593022, // P3s_W1s_N4c_T4c_B4c_U2h_W2h_unk_U2h_unk
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
        0x64593025, // P3s_W1s_N4c_T4c_B4c_U2h_W2h_unk_U2h_unk
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
        0x14d40022, // P3s_W1s_N4c_T4c_B4c_U2h_W2h
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
        0x14d4001f, // P3s_W1s_N4c_T4c_B4c_U2h_W2h
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
        0x77d87021, // P3s_W1s_N4c_T4c_B4c_U2h_W2h_VC4c
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
        0x77d87024, // P3s_W1s_N4c_T4c_B4c_U2h_W2h_VC4c
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
        0x207d6036, // P3f_N4c_U2h_VC4c
        BuildVertices(VertexPosition,
                      V{F::UINT, D::R32, U::Undefined},// polar ts?
                      TexCoord,
                      VertexColor),
    },
    {
        0x207d603b, // P3f_N4c_U2h_VC4c
        BuildVertices(VertexPosition,
                      V{F::UINT, D::R32, U::Undefined},// polar ts?
                      TexCoord,
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
        0x49b4f028, // P3f_N4c_T4c_U2h_VC4c
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      VertexColor),
    },
    {
        0x49b4f02d, // P3f_N4c_T4c_U2h_VC4c
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      VertexColor),
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
        0x49b4f00e, // P3f_N4c_T4c_U2h_VC4c
        BuildVertices(VertexPosition,
                      VertexNormalSigned,
                      VertexTangentSigned,
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
        0x49b4f015, // P3f_N4c_T4c_U2h_VC4c
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
        0x5e7f2030, // P3f_N4c_T4c_U2h_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
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
        0x926fd02d, // P3f_N4c_T4c_U2h_U2h_VC4c
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      VertexColor),
    },
    {
        0x926fd032, // P3f_N4c_T4c_U2h_U2h_VC4c
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      VertexColor),
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
        0xCBCF7026, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c_unk1i_U2h_unk1i
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
        0xCBCF702A, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c_unk1i_U2h_unk1i
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
        0xbb424023, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c
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
        0xbb424027, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c
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
        0xb3921020, // P3s_W1s_N4c_T4c_B2h_U2h_U2h_unk2i
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
        0xda55a020, // P3s_W1s_N4c_T4c_B4c_U2h_W2s_U2h
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
        0xda55a023, // P3s_W1s_N4c_T4c_B4c_U2h_W2s_U2h
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
        0xd9e801e, // P3s_W1s_N4c_T4c_U2h_B2h_U2h
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneIndices},
                      TexCoord),
    },
    {
        0xc31f201b, // P3s_W1s_N4c_T4c_U2h_B2h
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneIndices}),
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
        0xa013501d, // P3s_W1s_N4c_T4c_U2h_B2h_VC4c
        BuildVertices(VertexQPosition,
                      V{F::NORM, D::R16, U::BoneWeights},
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      V{F::FLOAT, D::R16G16, U::BoneIndices},
                      VertexColor),
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
        0xd877801a, // P3s_B1s_N4c_T4c_U2h_unk1i_U2h_unk1i
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
        0xcbf6c019, // P3s_unk1s_T4c_N4c_U2h_VC4c
        BuildVertices(VertexQPosition,
                      V{F::UINT, D::R16, U::Undefined}, // bone?
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      VertexColor),
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
        0xcbf6c00e, // P3s_unk1s_T4c_N4c_U2h_VC4c
        BuildVertices(VertexQPosition,
                      V{F::UINT, D::R16, U::Undefined},
                      VertexNormalSigned,
                      TexCoord,
                      TexCoord,
                      VertexColor,
                      VertexTangentSigned),
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
        0xafa63031, // P3f_N4c_T4c_U2h_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord),
    },
    {
        0xd8297027, // P3f_N4c_T4c_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord),
    },
    {
        0xd829702c, // P3f_N4c_T4c_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
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
        0xc66fa03e, // P3f_N4c_U2h_U2h
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
        0xd1a4703c, // P3f_N4c_U2h_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      TexCoord,
                      TexCoord),
    },
    {
        0xa7d7d035, // P3f_N4c_U2h
        BuildVertices(VertexPosition,
                      V{F::UINT, D::R32, U::Undefined},// polar ts?
                      TexCoord),
    },
    {
        0xa7d7d036, // P3f_N4c_U2h
        BuildVertices(VertexPosition,
                      VertexNormal,
                      TexCoord),
    },
    {
        0xa7d7d03a, // P3f_N4c_U2h
        BuildVertices(VertexPosition,
                      V{F::UINT, D::R32, U::Undefined},// polar ts?
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
        0xa14e0040, // P3f_N4c_U2h_U2h_VC4c
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
        0x9399c037, // P3f_N4c_T4c_U2h_U2h_VC4c
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
        0xb6681038, // P3f_N4c_T4c_U2h_U2h_VC4c_U2h
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
        0x63b6c033, // P3f_N4c_T4c_U2h_U2h_U2h_unk1i
        BuildVertices(VertexPosition,
                      VertexNormal,
                      VertexTangent,
                      TexCoord,
                      TexCoord,
                      TexCoord,
                      V{F::UINT, D::R32, U::Undefined}),
    },
    {
        0xa8fab017, // P3f_1s_N4c_T4c_U2h
        BuildVertices(VertexQPosition,
                      V{F::INT, D::R16, U::BoneIndices},
                      VertexNormal,
                      VertexTangent,
                      TexCoord),
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
    {
        0x667B1018, // P3s_unk1s_N4c_VC4c_U2h_T4c
        BuildVertices(VertexQPosition,
                      V{F::UINT, D::R16, U::Undefined},
                      VertexNormal,
                      VertexColor,
                      TexCoord,
                      VertexTangent),
    },
    {
        0x667B1019, // P3s_unk1s_N4c_VC4c_U2h_T4c
        BuildVertices(VertexQPosition,
                      V{F::UINT, D::R16, U::Undefined},
                      VertexNormal,
                      VertexColor,
                      TexCoord,
                      VertexTangent),
    },
    {
      0xF606F017,
      BuildVertices(
        VertexPosition,
        VertexNormalSigned,
        V{F::FLOAT, D::R32G32, U::TextureCoordiante},
        V{F::UINT, D::R8G8, U::BoneIndices},
        V{F::UNORM, D::R8G8, U::BoneWeights},
        VertexTangentSigned,
        V{F::UINT, D::R8G8, U::BoneIndices},
        V{F::UNORM, D::R8G8, U::BoneWeights}
      ),
    },
    {
      0x01B36016,
      BuildVertices(
        VertexPosition,
        VertexNormalSigned,
        V{F::FLOAT, D::R32G32, U::TextureCoordiante},
        V{F::UINT, D::R8G8, U::BoneIndices},
        V{F::UNORM, D::R8G8, U::BoneWeights},
        VertexTangentSigned, // always zero?
        V{F::UINT, D::R8G8, U::BoneIndices},
        V{F::UNORM, D::R8G8, U::BoneWeights}
      ),
    },
    {
      0xD6784014,
      BuildVertices(
        VertexPosition,
        VertexNormalSigned,
        V{F::FLOAT, D::R32G32, U::TextureCoordiante},
        V{F::UINT, D::R8G8, U::BoneIndices},
        V{F::UNORM, D::R8G8, U::BoneWeights}
      ),
    },
    {
      0x82917009,
      BuildVertices(
        VertexPosition,
        VertexNormalSigned,
        V{F::FLOAT, D::R32G32, U::TextureCoordiante}
      ),
    },
    {
      0x59DC400B,
      BuildVertices(
        VertexPosition,
        VertexNormalSigned,
        V{F::FLOAT, D::R32G32, U::TextureCoordiante},
        VertexTangentSigned
      ),
    },
    {
      0x43FB3015,
      BuildVertices(
        VertexPosition,
        VertexNormalSigned,
        V{F::FLOAT, D::R32G32, U::TextureCoordiante},
        V{F::UINT, D::R8G8, U::BoneIndices},
        V{F::UNORM, D::R8G8, U::BoneWeights},
        VertexTangentSigned
      ),
    },
    {
      0x7BA7401B,
      BuildVertices(
        VertexPosition,
        VertexNormalSigned,
        V{F::FLOAT, D::R32G32, U::TextureCoordiante},
        V{F::UINT, D::R8G8, U::BoneIndices},
        V{F::UNORM, D::R8G8, U::BoneWeights},
        VertexColor
      ),
    },
    {
      0xAE252019,
      BuildVertices(
        VertexPosition,
        VertexNormalSigned,
        V{F::FLOAT, D::R32G32, U::TextureCoordiante},
        V{F::UINT, D::R8G8, U::BoneIndices},
        V{F::UNORM, D::R8G8, U::BoneWeights},
        VertexTangentSigned,
        V{F::UINT, D::R8G8, U::BoneIndices},
        V{F::UNORM, D::R8G8, U::BoneWeights},
        V{F::UINT, D::R8G8, U::BoneIndices},
        V{F::UNORM, D::R8G8, U::BoneWeights},
        V{F::UINT, D::R8G8, U::BoneIndices},
        V{F::UNORM, D::R8G8, U::BoneWeights}
      ),
    },
    {
      0x3D62B012,
      BuildVertices(
        VertexPosition,
        VertexNormalSigned,
        V{F::FLOAT, D::R32G32, U::TextureCoordiante},
        VertexColor,
        VertexTangentSigned, // always zero?
        V{F::FLOAT, D::R32G32, U::TextureCoordiante}
      ),
    },
    {
      0x6180300A,
      BuildVertices(
        VertexPosition,
        VertexNormalSigned,
        V{F::FLOAT, D::R32G32, U::TextureCoordiante},
        VertexColor,
        VertexTangentSigned
      ),
    },
    {
      0x3F78800C,
      BuildVertices(
        VertexPosition,
        VertexNormalSigned,
        V{F::FLOAT, D::R32G32, U::TextureCoordiante},
        VertexColor,
        VertexTangentSigned
      ),
    },
    {
      0xEDF6F03C,
      BuildVertices(
        VertexPosition,
        VertexNormal,
        VertexTangent,
        VertexBoneIndices,
        V{F::UNORM, D::R8G8B8A8, U::BoneWeights},
        TexCoord
      ),
    },
    {
      0x3339D03A,
      BuildVertices(
        VertexPosition,
        VertexNormal3,
        V{F::UINT, D::R8, U::BoneIndices},
        TexCoord
      ),
    },
    {
      0xFD35504D,
      BuildVertices(
        VertexPosition,
        VertexNormal,
        Packed(VertexColor),
        TexCoord,
        TexCoord
      ),
    },
    {
      0x682EF04C,
      BuildVertices(
        VertexPosition,
        VertexNormal,
        Packed(VertexColor),
        TexCoord
      ),
    },
    {
      0x707FB01C,
      BuildVertices(
        VertexQPosition,
        V{F::UINT, D::R16, U::Undefined}, // bone id?
        VertexNormal,
        VertexTangent,
        TexCoord,
        V{F::UINT, D::R32, U::Undefined},
        V{F::UINT, D::R32, U::Undefined},
        TexCoord,
        VertexColor
      ),
    },
    {
      0xCC510026,
      BuildVertices(
        VertexQPosition,
        V{F::NORM, D::R16, U::BoneWeights},
        VertexNormal,
        VertexTangent,
        VertexBoneIndices,
        TexCoord,
        V{F::FLOAT, D::R16G16, U::BoneWeights},
        V{F::UINT, D::R32, U::Undefined},
        V{F::UINT, D::R32, U::Undefined},
        TexCoord,
        V{F::UINT, D::R32, U::Undefined}
      ),
    },
    {
      0x1B9A2021,
      BuildVertices(
        VertexQPosition,
        V{F::NORM, D::R16, U::BoneWeights},
        VertexNormal,
        VertexTangent,
        V{F::FLOAT, D::R16G16, U::BoneIndices},
        TexCoord,
        V{F::UINT, D::R32, U::Undefined},
        V{F::UINT, D::R32, U::Undefined},
        TexCoord,
        V{F::UINT, D::R32, U::Undefined}
      ),
    }
};

// clang-format on

static const std::set<uint32> edgeModels{
    0xdb7da014,
    0xdb7da013,
    // P3s_unk1s_B4c_W4c_N4c
    0x0CB68015,
    0x0CB68014,
    // P3s_B1s_N4c
    0xB0983013,
    // P3s_unk1s_B8c_W8c_N4c
    0xA320C015,
    0xA320C016,
    0xB0983014,
    0xB0983012,
};

static const auto makeV2 = [](auto &self, revil::MODImpl &main, auto &&fd) {
  MODPrimitiveProxy retval;
  uint8 visibleLOD = self.data0.template Get<MODMeshXC5::VisibleLOD>();
  retval.lodIndex =
      convertLod(reinterpret_cast<es::Flags<uint8> &>(visibleLOD));
  retval.materialIndex = self.data0.template Get<MODMeshXC5::MaterialIndex>();
  const MODMeshXC5::PrimitiveType_e primitiveType = MODMeshXC5::PrimitiveType_e(
      self.data1.template Get<
          typename std::decay_t<decltype(self)>::PrimitiveType>());

  switch (primitiveType) {
  case MODMeshXC5::PrimitiveType_e::Triangles:
    retval.indexType = uni::Primitive::IndexType_e::Triangle;
    break;

  case MODMeshXC5::PrimitiveType_e::Strips:
    retval.indexType = uni::Primitive::IndexType_e::Strip;
    break;

  default:
    retval.indexType = uni::Primitive::IndexType_e::None;
    break;
  }
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
    if (foundFormat->second.vertexStride != vertexStride) {
      foundFormat = fallbackFormats.find(self.vertexFormat);
      if (es::IsEnd(fallbackFormats, foundFormat)) {
        throw std::runtime_error("Cannot find fallback vertex format: " +
                                 std::to_string(self.vertexFormat));
      }
    }

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

    if (!edgeModels.contains(self.vertexFormat)) {
      throw std::runtime_error("Unregistered vertex format: " +
                               std::to_string(self.vertexFormat));
    }
  }

  uint16 *indexBuffer = reinterpret_cast<uint16 *>(
      &main.buffer[0] + main.vertexBufferSize + (self.indexStart * 2));

  MODIndices idArray;
  idArray.indexData = reinterpret_cast<const char *>(indexBuffer);
  idArray.numIndices = self.numIndices;

  for (size_t i = 0; i < self.numIndices; i++) {
    if (indexBuffer[i] != 0xffff) {
      indexBuffer[i] -= self.vertexStart;
    }
  }

  main.indices.storage.emplace_back(idArray);

  return retval;
};

MODPrimitiveProxy MODMeshXD2::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXD2> &>(main_);
  return makeV2(*this, main, [&](MODVertexDescriptor &d) {
    if (d.usage == uni::PrimitiveDescriptor::Usage_e::BoneIndices &&
        skinBoneBegin) {
      d.unpackType = uni::PrimitiveDescriptor::UnpackDataType_e::Add;
      d.unpackData.min = Vector4A16(skinBoneBegin);
    }
  });
}

MODPrimitiveProxy MODMeshXD2::ReflectBE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXD2> &>(main_);
  return makeV2(*this, main, [&](MODVertexDescriptor &d) {
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
  return makeV2(*this, main, [&](MODVertexDescriptor &d) {
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

MODPrimitiveProxy MODMeshXD3::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXD2> &>(main_);
  return makeV2(*this, main, [&](MODVertexDescriptor &d) {
    if (d.usage == uni::PrimitiveDescriptor::Usage_e::BoneIndices &&
        skinBoneBegin) {
      d.unpackType = uni::PrimitiveDescriptor::UnpackDataType_e::Add;
      d.unpackData.min = Vector4A16(skinBoneBegin);
    }
  });
}

MODPrimitiveProxy MODMeshXC5::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXC5> &>(main_);
  return makeV2(*this, main, [&](MODVertexDescriptor &) {});
}

MODPrimitiveProxy MODMeshXC5::ReflectBE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXC5> &>(main_);
  return makeV2(*this, main,
                [&](MODVertexDescriptor &d) { swapBuffers(d, numVertices); });
}

MODPrimitiveProxy MODMeshX06::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXD2> &>(main_);
  auto retval = makeV2(*this, main, [&](MODVertexDescriptor &) {});
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

MODPrimitiveProxy MODMeshXE5::ReflectBE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXD2> &>(main_);
  auto retval = makeV2(*this, main, [&](MODVertexDescriptor &d) {
    swapBuffers(d, numVertices);
  });
  retval.skinIndex = numEnvelopes;

  return retval;
}

MODPrimitiveProxy MODMeshXE5::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXD2> &>(main_);
  auto retval = makeV2(*this, main, [&](MODVertexDescriptor &) {});
  retval.skinIndex = numEnvelopes;

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
