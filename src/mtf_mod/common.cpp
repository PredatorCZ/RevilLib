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

using namespace revil;

struct AttributeAdd : AttributeCodec {
  void Sample(uni::FormatCodec::fvec &, const char *, size_t) const override {}
  void Transform(uni::FormatCodec::fvec &in) const override {
    for (Vector4A16 &v : in) {
      v += add;
    }
  }
  bool CanSample() const override { return false; }
  bool CanTransform() const override { return true; }
  bool IsNormalized() const override { return false; }

  uint32 add;
} ATTRADDCDX;

using F = uni::FormatType;
using D = uni::DataType;
using U = AttributeType;
using V = Attribute;

static AttributeUnormToSnorm NORMALCDX;

static const Attribute VertexNormal{D::R8G8B8A8, F::UNORM, U::Normal, -1,
                                    &NORMALCDX};
static const Attribute VertexNormal3{D::R8G8B8, F::UNORM, U::Normal, -1,
                                     &NORMALCDX};
static const Attribute VertexTangent{D::R8G8B8A8, F::UNORM, U::Tangent, -1,
                                     &NORMALCDX};
static const Attribute VertexTangent3{D::R8G8B8, F::UNORM, U::Tangent, -1,
                                      &NORMALCDX};

struct AttributeTexCoordPhone : AttributeCodec {
  void Sample(uni::FormatCodec::fvec &, const char *, size_t) const override {}
  void Transform(uni::FormatCodec::fvec &in) const override {
    for (Vector4A16 &v : in) {
      v *= 64;
    }
  }
  bool CanSample() const override { return false; }
  bool CanTransform() const override { return true; }
  bool IsNormalized() const override { return false; }
} TXCRDPHONECDX;

static const Attribute TexCoordPhone{D::R16G16, F::UNORM, U::TextureCoordiante,
                                     -1, &TXCRDPHONECDX};

Attribute Packed(Attribute in) {
  in.offset = 1;
  return in;
}

static const Attribute TexCoord{D::R16G16, F::FLOAT, U::TextureCoordiante};

static const Attribute VertexQPosition{D::R16G16B16, F::NORM, U::Position};

static const Attribute VertexPosition{D::R32G32B32, F::FLOAT, U::Position};

static const Attribute VertexBoneIndices{D::R8G8B8A8, F::UINT, U::BoneIndices};

static const Attribute VertexColor{D::R8G8B8A8, F::UNORM, U::VertexColor};

static const Attribute VertexNormalSigned{D::R8G8B8A8, F::NORM, U::Normal};

static const Attribute VertexTangentSigned{D::R8G8B8A8, F::NORM, U::Tangent};

static constexpr uint32 fmtStrides[]{0,  128, 96, 64, 64, 48, 32, 32, 32,
                                     32, 32,  32, 24, 16, 16, 16, 16, 8};

static const auto swapBuffers = [](MODVertexSpan &spn) {
  const size_t numVertices = spn.numVertices;
  uint32 stride = spn.stride;
  uint32 curOffset = 0;

  for (auto &d : spn.attrs) {
    char *curBuffer = spn.buffer + curOffset;
    curOffset += fmtStrides[uint32(d.type)] / 8;

    switch (d.type) {
    case uni::DataType::R16: {
      for (size_t v = 0; v < numVertices; v++, curBuffer += stride) {
        FByteswapper(*reinterpret_cast<uint16 *>(curBuffer));
      }
      break;
    }

    case uni::DataType::R16G16: {
      for (size_t v = 0; v < numVertices; v++, curBuffer += stride) {
        FByteswapper(*reinterpret_cast<USVector2 *>(curBuffer));
      }
      break;
    }

    case uni::DataType::R16G16B16: {
      for (size_t v = 0; v < numVertices; v++, curBuffer += stride) {
        FByteswapper(*reinterpret_cast<USVector *>(curBuffer));
      }
      break;
    }

    case uni::DataType::R16G16B16A16: {
      for (size_t v = 0; v < numVertices; v++, curBuffer += stride) {
        FByteswapper(*reinterpret_cast<USVector4 *>(curBuffer));
      }
      break;
    }

    case uni::DataType::R32:
    case uni::DataType::R10G10B10A2: {
      for (size_t v = 0; v < numVertices; v++, curBuffer += stride) {
        FByteswapper(*reinterpret_cast<uint32 *>(curBuffer));
      }
      break;
    }

    case uni::DataType::R8G8B8A8:
      if (d.offset == 1) {
        for (size_t v = 0; v < numVertices; v++, curBuffer += stride) {
          FByteswapper(*reinterpret_cast<uint32 *>(curBuffer));
        }
      }
      break;

    case uni::DataType::R32G32: {
      for (size_t v = 0; v < numVertices; v++, curBuffer += stride) {
        FByteswapper(*reinterpret_cast<Vector2 *>(curBuffer));
      }
      break;
    }

    case uni::DataType::R32G32B32: {
      for (size_t v = 0; v < numVertices; v++, curBuffer += stride) {
        FByteswapper(*reinterpret_cast<Vector *>(curBuffer));
      }
      break;
    }

    default:
      break;
    }
  }
};

static const auto makeVertices0X70 = [](uint8 useSkin, bool v1stride4,
                                        auto &main) {
  std::vector<Attribute> attrs;
  std::vector<std::unique_ptr<AttributeCodec>> codecs;
  const bool skin8 = useSkin == 2;

  if (useSkin) {
    auto mcdx = std::make_unique<AttributeMad>();
    mcdx->mul = main.bounds.bboxMax - main.bounds.bboxMin;
    mcdx->add = main.bounds.bboxMin;
    attrs.emplace_back(
        Attribute{D::R16G16B16A16, F::UNORM, U::Position, -1, mcdx.get()});
    codecs.emplace_back(std::move(mcdx));

    if (skin8) {
      attrs.emplace_back(VertexBoneIndices);
      attrs.emplace_back(VertexBoneIndices);
      attrs.emplace_back(D::R8G8B8A8, F::UNORM, U::BoneWeights);
      attrs.emplace_back(D::R8G8B8A8, F::UNORM, U::BoneWeights);
      attrs.emplace_back(D::R10G10B10A2, F::NORM, U::Normal);
      attrs.emplace_back(TexCoord);
    } else {
      attrs.emplace_back(VertexBoneIndices);
      attrs.emplace_back(D::R8G8B8A8, F::UNORM, U::BoneWeights);
      attrs.emplace_back(D::R10G10B10A2, F::NORM, U::Normal);
      attrs.emplace_back(D::R10G10B10A2, F::NORM, U::Undefined); // tangent
      attrs.emplace_back(TexCoord);
    }
  } else {
    attrs.emplace_back(VertexPosition);
    attrs.emplace_back(Packed(VertexNormalSigned));

    if (v1stride4) {
      // attrs.emplace_back(Packed(VertexTangentSigned)); // tangent
      attrs.emplace_back(D::R32, F::UINT, U::Undefined);
      attrs.emplace_back(TexCoord);
      attrs.emplace_back(TexCoord);
      // null or swapped with tangent
      attrs.emplace_back(D::R32, F::UINT, U::Undefined);
    } else {
      // always null?
      attrs.emplace_back(D::R32, F::UINT, U::Undefined);
      attrs.emplace_back(TexCoord);
      attrs.emplace_back(TexCoord);
      // attrs.emplace_back(Packed(VertexTangentSigned));
      attrs.emplace_back(D::R32, F::UINT, U::Undefined);
    }
  }

  return std::make_pair(attrs, std::move(codecs));
};

static const auto makeVertices1X70 = [](uint8 useSkin, bool v1stride4) {
  std::vector<Attribute> attrs;
  const bool skin8 = useSkin == 2;

  if (skin8) {
    // attrs.emplace_back(D::R10G10B10A2, F::NORM, U::Tangent);
    attrs.emplace_back(D::R32, F::UINT, U::Undefined);
    attrs.emplace_back(TexCoord);
  } else {
    attrs.emplace_back(VertexColor);

    if (!v1stride4) {
      attrs.emplace_back(VertexColor);
    }
  }
  return attrs;
};

static const auto makeVertices0X99 = [](uint8 useSkin, bool v1stride4,
                                        auto &main) {
  std::vector<Attribute> attrs;
  std::vector<std::unique_ptr<AttributeCodec>> codecs;
  const bool skin8 = useSkin == 4;

  if (useSkin) {
    auto mcdx = std::make_unique<AttributeMad>();
    mcdx->mul = main.bounds.bboxMax - main.bounds.bboxMin;
    mcdx->add = main.bounds.bboxMin;
    attrs.emplace_back(
        Attribute{D::R16G16B16A16, F::NORM, U::Position, -1, mcdx.get()});
    codecs.emplace_back(std::move(mcdx));

    if (skin8) {
      attrs.emplace_back(VertexBoneIndices);
      attrs.emplace_back(VertexBoneIndices);
      attrs.emplace_back(D::R8G8B8A8, F::UNORM, U::BoneWeights);
      attrs.emplace_back(D::R8G8B8A8, F::UNORM, U::BoneWeights);
      attrs.emplace_back(VertexNormal);
      attrs.emplace_back(TexCoord);
    } else {
      attrs.emplace_back(VertexBoneIndices);
      attrs.emplace_back(D::R8G8B8A8, F::UNORM, U::BoneWeights);
      attrs.emplace_back(VertexNormal);
      // attrs.emplace_back(VertexTangent);
      attrs.emplace_back(D::R32, F::UINT, U::Undefined);
      attrs.emplace_back(TexCoord);
    }
  } else {
    attrs.emplace_back(VertexPosition);
    attrs.emplace_back(VertexNormal);

    if (v1stride4) {
      // attrs.emplace_back(VertexTangent);
      attrs.emplace_back(D::R32, F::UINT, U::Undefined);
      attrs.emplace_back(TexCoord);
      attrs.emplace_back(TexCoord);
      // null or swapped with tangent
      attrs.emplace_back(D::R32, F::UINT, U::Undefined);
    } else {
      // always null?
      attrs.emplace_back(D::R32, F::UINT, U::Undefined);
      attrs.emplace_back(TexCoord);
      attrs.emplace_back(TexCoord);
      // attrs.emplace_back(VertexTangent);
      attrs.emplace_back(D::R32, F::UINT, U::Undefined);
    }
  }

  return std::make_pair(attrs, std::move(codecs));
};

static const auto makeVertices1X99 = [](uint8 useSkin, bool v1stride4) {
  std::vector<Attribute> attrs;
  const bool skin8 = useSkin == 4;

  if (skin8) {
    // attrs.emplace_back(VertexTangent);
    attrs.emplace_back(D::R32, F::UINT, U::Undefined);
    attrs.emplace_back(TexCoord);
  } else {
    attrs.emplace_back(VertexColor);

    if (!v1stride4) {
      attrs.emplace_back(VertexColor);
    }
  }
  return attrs;
};

static const auto makeVertices0X170 = [](uint8 useSkin, bool, auto &main) {
  std::vector<Attribute> attrs;
  std::vector<std::unique_ptr<AttributeCodec>> codecs;
  const bool skin8 = useSkin == 2;

  if (useSkin) {
    auto mcdx = std::make_unique<AttributeMad>();
    mcdx->mul = main.bounds.bboxMax - main.bounds.bboxMin;
    mcdx->add = main.bounds.bboxMin;
    attrs.emplace_back(
        Attribute{D::R16G16B16A16, F::NORM, U::Position, -1, mcdx.get()});
    codecs.emplace_back(std::move(mcdx));

    if (skin8) {
      attrs.emplace_back(VertexBoneIndices);
      attrs.emplace_back(VertexBoneIndices);
      attrs.emplace_back(D::R8G8B8A8, F::UNORM, U::BoneWeights);
      attrs.emplace_back(D::R8G8B8A8, F::UNORM, U::BoneWeights);
      attrs.emplace_back(VertexNormal);
      attrs.emplace_back(TexCoord);
    } else {
      attrs.emplace_back(VertexBoneIndices);
      attrs.emplace_back(D::R8G8B8A8, F::UNORM, U::BoneWeights);
      attrs.emplace_back(D::R16G16B16A16, F::NORM, U::Normal);
      attrs.emplace_back(TexCoord);
    }
  } else {
    attrs.emplace_back(VertexPosition);
    attrs.emplace_back(D::R16G16B16A16, F::NORM, U::Normal);
    attrs.emplace_back(D::R32G32, F::FLOAT, U::TextureCoordiante);
  }

  return std::make_pair(attrs, std::move(codecs));
};

static const auto makeVertices1X170 = [](uint8 useSkin, bool) {
  std::vector<Attribute> attrs;
  const bool skin8 = useSkin == 2;

  if (useSkin) {
    // attrs.emplace_back(D::R16G16B16A16, F::NORM, U::Tangent);
    attrs.emplace_back(D::R32G32, F::UINT, U::Undefined);
    attrs.emplace_back(TexCoord);
  } else if (skin8) {
    // attrs.emplace_back(VertexTangent);
    attrs.emplace_back(D::R32, F::UINT, U::Undefined);
    attrs.emplace_back(TexCoord);
  } else {
    // attrs.emplace_back(D::R16G16B16A16, F::NORM, U::Tangent);
    attrs.emplace_back(D::R32G32, F::UINT, U::Undefined);
    attrs.emplace_back(D::R32G32, F::FLOAT, U::TextureCoordiante);
    attrs.emplace_back(VertexColor);
    attrs.emplace_back(VertexColor);
    attrs.emplace_back(VertexColor);
  }
  return attrs;
};

static const auto makeV1 = [](auto &self, auto &main, auto v0Maker,
                              auto v1Maker, auto &&fd) {
  auto &mat = main.materials[self.materialIndex].main;
  using material_type = typename std::remove_reference<decltype(mat)>::type;
  auto skinType = mat.vshData.template Get<typename material_type::SkinType>();

  revil::MODPrimitive retval;
  retval.lod1 = self.visibleLOD[0];
  retval.lod2 = self.visibleLOD[1];
  retval.lod3 = self.visibleLOD[2];
  retval.materialIndex = self.materialIndex;
  retval.triStrips = true;
  retval.groupId = self.unk;
  retval.meshId = 0;
  retval.vertexIndex = main.vertices.size();

  MODVertexSpan vtx;
  std::tie(vtx.attrs, vtx.codecs) =
      v0Maker(skinType, self.buffer1Stride != 8, main);
  vtx.numVertices = self.numVertices;
  vtx.buffer =
      main.vertexBuffer.data() + (self.vertexStart * self.buffer0Stride) +
      self.vertexStreamOffset + (self.indexValueOffset * self.buffer0Stride);
  vtx.stride = self.buffer0Stride;
  fd(vtx);
  for (auto &a : vtx.attrs) {
    a.offset = -1;
  }

  main.vertices.emplace_back(std::move(vtx));

  /*if (skin8 && self.buffer1Stride != 8) {
    throw std::runtime_error("Expected secondary buffer of 8 bytes!");
  }

  if (!skin8 && skinType && self.buffer1Stride) {
    throw std::runtime_error("Unexpected secondary buffer for skin!");
  }*/

  if (self.buffer1Stride) {
    MODVertexSpan vtx1;
    vtx1.attrs = v1Maker(skinType, self.buffer1Stride != 8);
    vtx1.numVertices = self.numVertices;
    vtx1.buffer = &main.vertexBuffer.back() - main.unkBufferSize + 1;
    vtx1.buffer += (self.vertexStart * self.buffer1Stride) +
                   self.vertexStream2Offset +
                   (self.indexValueOffset * self.buffer1Stride);
    vtx1.stride = self.buffer1Stride;
    fd(vtx1);
    for (auto &a : vtx1.attrs) {
      a.offset = -1;
    }

    main.vertices.emplace_back(std::move(vtx1));
  }

  uint16 *indexBuffer = main.indexBuffer.data() + self.indexStart;
  retval.indexIndex = main.indices.size();

  MODIndexSpan idArray(indexBuffer, self.numIndices);

  for (auto &idx : idArray) {
    if (idx != 0xffff) {
      idx -= self.vertexStart;
    }
  }

  main.indices.emplace_back(idArray);

  return retval;
};

revil::MODPrimitive MODMeshX70::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsX170> &>(main_);
  return makeV1(*this, main, makeVertices0X170, makeVertices1X170,
                [&](MODVertexSpan &) {});
}

revil::MODPrimitive MODMeshX70::ReflectBE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsX70> &>(main_);
  return makeV1(*this, main, makeVertices0X70, makeVertices1X70,
                [&](MODVertexSpan &d) { swapBuffers(d); });
}

revil::MODPrimitive MODMeshX99::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsX99LE> &>(main_);
  auto retval = makeV1(*this, main, makeVertices0X99, makeVertices1X99,
                       [&](MODVertexSpan &) {});
  retval.skinIndex = skinInfo.boneRemapIndex;
  return retval;
}

revil::MODPrimitive MODMeshX99::ReflectBE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsX99BE> &>(main_);
  auto retval = makeV1(*this, main, makeVertices0X99, makeVertices1X99,
                       [&](MODVertexSpan &d) { swapBuffers(d); });
  retval.skinIndex = skinInfo.boneRemapIndex;
  return retval;
}

struct MODAttributes {
  std::vector<Attribute> attrs;
  uint32 stride = 0;

  MODAttributes(std::initializer_list<Attribute> attrs_) : attrs(attrs_) {
    for (auto &d : attrs) {
      stride += fmtStrides[uint32(d.type)];
    }

    stride /= 8;
  }
};

std::map<uint32, MODAttributes> fallbackFormats{
    {
        // xc3
        0xc31f2014, // P3s_W1s_N3c_B1c_T3c_B1c_U2h
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal3,
            V{D::R8, F::UINT, U::BoneIndices},
            VertexTangent3,
            V{D::R8, F::UINT, U::BoneIndices},
            TexCoord,
        },
    },
    {
        // xd2 be, this might be correct after all
        0xd8297028, // P3f_N4c_T4c_U2h
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            VertexColor,
        },
    },
};

std::map<uint32, MODAttributes> formats{
    {
        0x64593022, // P3s_W1s_N4c_T4c_B4c_U2h_W2h_unk_U2h_unk
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            V{D::R32, F::UINT, U::Undefined},
            TexCoord,
            V{D::R32, F::UINT, U::Undefined},
        },
    },
    {
        0x64593023, // P3s_W1s_N4c_T4c_B4c_U2h_W2h_unk_U2h_unk
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            V{D::R32, F::UINT, U::Undefined},
            TexCoord,
            V{D::R32, F::UINT, U::Undefined},
        },
    },
    {
        0x64593025, // P3s_W1s_N4c_T4c_B4c_U2h_W2h_unk_U2h_unk
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            V{D::R32, F::UINT, U::Undefined},
            TexCoord,
            V{D::R32, F::UINT, U::Undefined},
        },
    },
    {
        0x14d40020, // P3s_W1s_N4c_T4c_B4c_U2h_W2h
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
        },
    },
    {
        0x14d40022, // P3s_W1s_N4c_T4c_B4c_U2h_W2h
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
        },
    },
    {
        0x14d40019, // P3s_W1s_N4c_T4c_B4c_U2h_W2h
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
        },
    },
    {
        0x14d4001f, // P3s_W1s_N4c_T4c_B4c_U2h_W2h
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
        },
    },
    {
        0x14d40021, // P3s_W1s_N4c_T4c_B4c_U2h_W2h
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormalSigned,
            VertexTangentSigned,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
        },
    },
    {
        0xbde5301a, // P3s_W1s_N4c_T4c_B4c_U2h_W2h
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
        },
    },
    {
        0xd86ca01c, // P3s_W1s_N4c_T4c_B4c_U2h_W2h_VC4c
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            VertexColor,
        },
    },
    {
        0x77d87021, // P3s_W1s_N4c_T4c_B4c_U2h_W2h_VC4c
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            VertexColor,
        },
    },
    {
        0x77d87024, // P3s_W1s_N4c_T4c_B4c_U2h_W2h_VC4c
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            VertexColor,
        },
    },
    {
        0x77d87022, // P3s_W1s_N4c_T4c_B4c_U2h_W2h_VC4c
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            VertexColor,
        },
    },
    {
        0x77d87023, // P3s_W1s_N4c_T4c_B4c_U2h_W2h_VC4c
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormalSigned,
            VertexTangentSigned,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            VertexColor,
        },
    },
    {
        0x207d6036, // P3f_N4c_U2h_VC4c
        {
            VertexPosition,
            V{D::R32, F::UINT, U::Undefined}, // polar ts?
            TexCoord,
            VertexColor,
        },
    },
    {
        0x207d603b, // P3f_N4c_U2h_VC4c
        {
            VertexPosition,
            V{D::R32, F::UINT, U::Undefined}, // polar ts?
            TexCoord,
            VertexColor,
        },
    },
    {
        0x207d6037, // P3f_N4c_U2h_VC4c
        {
            VertexPosition,
            VertexNormal,
            TexCoord,
            VertexColor,
        },
    },
    {
        0x207d6030, // P3f_N4c_U2h_VC4c
        {
            VertexPosition,
            VertexNormal,
            TexCoord,
            VertexColor,
        },
    },
    {
        0x207d6038, // P3f_N4c_U2h_VC4c
        {
            VertexPosition,
            VertexNormalSigned,
            TexCoord,
            VertexColor,
        },
    },
    {
        0x2f55c03d, // P3s_W1s_N4c_T4c_B4c_U2h_W2h_unk36c
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            V{D::R32, F::UINT, U::Undefined},
            V{D::R32G32B32A32, F::UINT, U::Undefined},
            V{D::R32G32B32A32, F::UINT, U::Undefined},
        },
    },
    {
        0x49b4f028, // P3f_N4c_T4c_U2h_VC4c
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            VertexColor,
        },
    },
    {
        0x49b4f02d, // P3f_N4c_T4c_U2h_VC4c
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            VertexColor,
        },
    },
    {
        0x49b4f029, // P3f_N4c_T4c_U2h_VC4c
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            VertexColor,
        },
    },
    {
        0x49b4f00e, // P3f_N4c_T4c_U2h_VC4c
        {
            VertexPosition,
            VertexNormalSigned,
            VertexTangentSigned,
            TexCoord,
            VertexColor,
        },
    },
    {
        0x49b4f022, // P3f_N4c_T4c_U2h_VC4c
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            VertexColor,
        },
    },
    {
        0x49b4f02a, // P3f_N4c_T4c_U2h_VC4c
        {
            VertexPosition,
            VertexNormalSigned,
            VertexTangentSigned,
            TexCoord,
            VertexColor,
        },
    },
    {
        0x49b4f015, // P3f_N4c_T4c_U2h_VC4c
        {
            VertexPosition,
            VertexNormalSigned,
            VertexTangentSigned,
            TexCoord,
            VertexColor,
        },
    },
    {
        0x5e7f202c, // P3f_N4c_T4c_U2h_U2h
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
        },
    },
    {
        0x5e7f202d, // P3f_N4c_T4c_U2h_U2h
        {
            VertexPosition,
            VertexNormalSigned,
            VertexTangentSigned,
            TexCoord,
            TexCoord,
        },
    },
    {
        0x5e7f2030, // P3f_N4c_T4c_U2h_U2h
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
        },
    },
    {
        0x747d1031, // P3f_N4c_T4c_U2h_U2h_U2h
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
            TexCoord,
        },
    },
    {
        0x75c3e025, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c_U2h
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            V{D::R8G8B8A8, F::UNORM, U::BoneWeights},
            VertexBoneIndices,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            VertexTangent,
            TexCoord,
        },
    },
    {
        0x926fd02d, // P3f_N4c_T4c_U2h_U2h_VC4c
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
            VertexColor,
        },
    },
    {
        0x926fd032, // P3f_N4c_T4c_U2h_U2h_VC4c
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
            VertexColor,
        },
    },
    {
        0x926fd02e, // P3f_N4c_T4c_U2h_U2h_VC4c
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
            VertexColor,
        },
    },
    {
        0xb86de02a, // P3f_N4c_T4c_U2h_U2h_VC4c
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
            VertexColor,
        },
    },
    {
        0xCBCF7026, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c_unk1i_U2h_unk1i
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            V{D::R8G8B8A8, F::UNORM, U::BoneWeights},
            VertexBoneIndices,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            VertexTangent,
            V{D::R32, F::UINT, U::Undefined},
            TexCoord,
            V{D::R32, F::UINT, U::Undefined},
        },
    },
    {
        0xCBCF702A, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c_unk1i_U2h_unk1i
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            V{D::R8G8B8A8, F::UNORM, U::BoneWeights},
            VertexBoneIndices,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            VertexTangent,
            V{D::R32, F::UINT, U::Undefined},
            TexCoord,
            V{D::R32, F::UINT, U::Undefined},
        },
    },
    {
        0xCBCF7027, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c_unk1i_U2h_unk1i
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            V{D::R8G8B8A8, F::UNORM, U::BoneWeights},
            VertexBoneIndices,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            VertexTangent,
            V{D::R32, F::UINT, U::Undefined},
            TexCoord,
            V{D::R32, F::UINT, U::Undefined},
        },
    },
    {
        0xbb424023, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            V{D::R8G8B8A8, F::UNORM, U::BoneWeights},
            VertexBoneIndices,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            VertexTangent,
        },
    },
    {
        0xbb424027, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            V{D::R8G8B8A8, F::UNORM, U::BoneWeights},
            VertexBoneIndices,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            VertexTangent,
        },
    },
    {
        0xbb424024, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            V{D::R8G8B8A8, F::UNORM, U::BoneWeights},
            VertexBoneIndices,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            VertexTangent,
        },
    },
    {
        0x1273701e, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            V{D::R8G8B8A8, F::UNORM, U::BoneWeights},
            VertexBoneIndices,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            VertexTangent,
        },
    },
    {
        0xbb42401d, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            V{D::R8G8B8A8, F::UNORM, U::BoneWeights},
            VertexBoneIndices,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            VertexTangent,
        },
    },
    {
        0xbb424025, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormalSigned,
            V{D::R8G8B8A8, F::UNORM, U::BoneWeights},
            VertexBoneIndices,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            VertexTangentSigned,
        },
    },
    {
        0xb392101f, // P3s_W1s_N4c_T4c_B2h_U2h_U2h_unk2i
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            V{D::R16G16, F::FLOAT, U::BoneIndices},
            TexCoord,
            TexCoord,
            V{D::R32G32, F::UINT, U::Undefined},
        },
    },
    {
        0xb3921020, // P3s_W1s_N4c_T4c_B2h_U2h_U2h_unk2i
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            V{D::R16G16, F::FLOAT, U::BoneIndices},
            TexCoord,
            TexCoord,
            V{D::R32G32, F::UINT, U::Undefined},
        },
    },
    {
        0xda55a020, // P3s_W1s_N4c_T4c_B4c_U2h_W2s_U2h
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            TexCoord,
        },
    },
    {
        0xda55a021, // P3s_W1s_N4c_T4c_B4c_U2h_W2s_U2h
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            TexCoord,
        },
    },
    {
        0xda55a023, // P3s_W1s_N4c_T4c_B4c_U2h_W2s_U2h
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            TexCoord,
        },
    },
    {
        0xd9e801d, // P3s_W1s_N4c_T4c_U2h_B2h_U2h
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneIndices},
            TexCoord,
        },
    },
    {
        0xd9e801e, // P3s_W1s_N4c_T4c_U2h_B2h_U2h
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneIndices},
            TexCoord,
        },
    },
    {
        0xc31f201b, // P3s_W1s_N4c_T4c_U2h_B2h
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneIndices},
        },
    },
    {
        0xc31f201c, // P3s_W1s_N4c_T4c_U2h_B2h
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneIndices},
        },
    },
    {
        0x6a2e1016, // P3s_W1s_N4c_T4c_U2h_B2h
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneIndices},
        },
    },
    {
        0xc31f2014, // P3s_W1s_N4c_T4c_U2h_B2h
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneIndices},
        },
    },
    {
        0xc31f201d, // P3s_W1s_N4c_T4c_U2h_B2h
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormalSigned,
            VertexTangentSigned,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneIndices},
        },
    },
    {
        0xa013501d, // P3s_W1s_N4c_T4c_U2h_B2h_VC4c
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneIndices},
            VertexColor,
        },
    },
    {
        0xa013501e, // P3s_W1s_N4c_T4c_U2h_B2h_VC4c
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneIndices},
            VertexColor,
        },
    },
    {
        0xa013501f, // P3s_W1s_N4c_T4c_U2h_B2h_VC4c
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormalSigned,
            VertexTangentSigned,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneIndices},
            VertexColor,
        },
    },
    {
        0xd877801a, // P3s_B1s_N4c_T4c_U2h_unk1i_U2h_unk1i
        {
            VertexQPosition,
            V{D::R16, F::UINT, U::BoneIndices},
            VertexNormal,
            VertexTangent,
            TexCoord,
            V{D::R32, F::UINT, U::Undefined},
            TexCoord,
            V{D::R32, F::UINT, U::Undefined},
        },
    },
    {
        0xd877801b, // P3s_B1s_N4c_T4c_U2h_unk1i_U2h_unk1i
        {
            VertexQPosition,
            V{D::R16, F::UINT, U::BoneIndices},
            VertexNormal,
            VertexTangent,
            TexCoord,
            V{D::R32, F::UINT, U::Undefined},
            TexCoord,
            V{D::R32, F::UINT, U::Undefined},
        },
    },
    {
        0xcbf6c019, // P3s_unk1s_T4c_N4c_U2h_VC4c
        {
            VertexQPosition,
            V{D::R16, F::UINT, U::Undefined}, // bone?
            VertexNormal,
            VertexTangent,
            TexCoord,
            VertexColor,
        },
    },
    {
        0xcbf6c01a, // P3s_unk1s_T4c_N4c_U2h_VC4c
        {
            VertexQPosition,
            V{D::R16, F::UINT, U::Undefined}, // bone?
            VertexTangent,
            VertexNormal,
            TexCoord,
            VertexColor,
        },
    },
    {
        0xcbf6c01b, // P3s_unk1s_T4c_N4c_U2h_VC4c
        {
            VertexQPosition,
            V{D::R16, F::UINT, U::Undefined}, // bone?
            VertexTangentSigned,
            VertexNormalSigned,
            TexCoord,
            VertexColor,
        },
    },
    {
        0xcbf6c00e, // P3s_unk1s_T4c_N4c_U2h_VC4c
        {
            VertexQPosition,
            V{D::R16, F::UINT, U::Undefined},
            VertexNormalSigned,
            TexCoord,
            TexCoord,
            VertexColor,
            VertexTangentSigned,
        },
    },
    {
        0x37a4e035, // P3s_N4c_T4c_U2h_U2h_U2h_U2h
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
            TexCoord,
            TexCoord,
        },
    },
    {
        0x12553032, // P3s_N4c_T4c_U2h_U2h_U2h
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
            TexCoord,
        },
    },
    {
        0xafa6302d, // P3f_N4c_T4c_U2h_U2h
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
        },
    },
    {
        0xafa63031, // P3f_N4c_T4c_U2h_U2h
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
        },
    },
    {
        0xd8297027, // P3f_N4c_T4c_U2h
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
        },
    },
    {
        0xd829702c, // P3f_N4c_T4c_U2h
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
        },
    },
    {
        0xd8297028, // P3f_N4c_T4c_U2h
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
        },
    },
    {
        0xd8297021, // P3f_N4c_T4c_U2h
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
        },
    },
    {
        0xd8297029, // P3f_N4c_T4c_U2h
        {
            VertexPosition,
            VertexNormalSigned,
            VertexTangentSigned,
            TexCoord,
        },
    },
    {
        0x2082f03b, // P3f_N4c_U2h_U2h_U2h
        {
            VertexPosition,
            VertexNormal,
            TexCoord,
            TexCoord,
            TexCoord,
        },
    },
    {
        0xc66fa03a, // P3f_N4c_U2h_U2h
        {
            VertexPosition,
            VertexNormal,
            TexCoord,
            TexCoord,
        },
    },
    {
        0xc66fa03e, // P3f_N4c_U2h_U2h
        {
            VertexPosition,
            VertexNormal,
            TexCoord,
            TexCoord,
        },
    },
    {
        0xd1a47038, // P3f_N4c_U2h_U2h
        {
            VertexPosition,
            VertexNormal,
            TexCoord,
            TexCoord,
        },
    },
    {
        0xd1a4703c, // P3f_N4c_U2h_U2h
        {
            VertexPosition,
            VertexNormal,
            TexCoord,
            TexCoord,
        },
    },
    {
        0xa7d7d035, // P3f_N4c_U2h
        {
            VertexPosition,
            V{D::R32, F::UINT, U::Undefined}, // polar ts?
            TexCoord,
        },
    },
    {
        0xa7d7d036, // P3f_N4c_U2h
        {
            VertexPosition,
            VertexNormal,
            TexCoord,
        },
    },
    {
        0xa7d7d03a, // P3f_N4c_U2h
        {
            VertexPosition,
            V{D::R32, F::UINT, U::Undefined}, // polar ts?
            TexCoord,
        },
    },
    {
        0xa7d7d02f, // P3f_N4c_U2h
        {
            VertexPosition,
            VertexNormal,
            TexCoord,
        },
    },
    {
        0xa7d7d037, // P3f_N4c_U2h
        {
            VertexPosition,
            VertexNormalSigned,
            TexCoord,
        },
    },
    {
        0xa14e003c, // P3f_N4c_U2h_U2h_VC4c
        {
            VertexPosition,
            VertexNormal,
            TexCoord,
            TexCoord,
            VertexColor,
        },
    },
    {
        0xa14e0040, // P3f_N4c_U2h_U2h_VC4c
        {
            VertexPosition,
            VertexNormal,
            TexCoord,
            TexCoord,
            VertexColor,
        },
    },
    {
        0x9399c033, // P3f_N4c_T4c_U2h_U2h_VC4c
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
            VertexColor,
        },
    },
    {
        0x9399c037, // P3f_N4c_T4c_U2h_U2h_VC4c
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
            VertexColor,
        },
    },
    {
        0x4325a03e, // P3f_N4c_T4c_U2h_U2h_unk3h_unk1s_unki7
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
            V{D::R32G32B32A32, F::UINT, U::Undefined},
            V{D::R32G32B32A32, F::UINT, U::Undefined},
            V{D::R32, F::UINT, U::Undefined},
        },
    },
    {
        0xb6681034, // P3f_N4c_T4c_U2h_U2h_VC4c_U2h
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
            VertexColor,
            TexCoord,
        },
    },
    {
        0xb6681038, // P3f_N4c_T4c_U2h_U2h_VC4c_U2h
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
            VertexColor,
            TexCoord,
        },
    },
    {
        0x63b6c02f, // P3f_N4c_T4c_U2h_U2h_U2h_unk1i
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
            TexCoord,
            V{D::R32, F::UINT, U::Undefined},
        },
    },
    {
        0x63b6c033, // P3f_N4c_T4c_U2h_U2h_U2h_unk1i
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
            TexCoord,
            V{D::R32, F::UINT, U::Undefined},
        },
    },
    {
        0xa8fab017, // P3f_1s_N4c_T4c_U2h
        {
            VertexQPosition,
            V{D::R16, F::UINT, U::BoneIndices},
            VertexNormal,
            VertexTangent,
            TexCoord,
        },
    },
    {
        0xa8fab018, // P3f_1s_N4c_T4c_U2h
        {
            VertexQPosition,
            V{D::R16, F::UINT, U::BoneIndices},
            VertexNormal,
            VertexTangent,
            TexCoord,
        },
    },
    {
        0xa8fab010, // P3f_1s_N4c_T4c_U2h
        {
            VertexQPosition,
            V{D::R16, F::UINT, U::BoneIndices},
            VertexNormal,
            VertexTangent,
            TexCoord,
        },
    },
    {
        0xa8fab019, // P3f_1s_N4c_T4c_U2h
        {
            VertexQPosition,
            V{D::R16, F::UINT, U::BoneIndices},
            VertexNormalSigned,
            VertexTangentSigned,
            TexCoord,
        },
    },
    {
        0x1cb8011, // P3s_B1s_N4c_T4c_U2h
        {
            VertexQPosition,
            V{D::R16, F::UINT, U::BoneIndices},
            VertexNormal,
            VertexTangent,
            TexCoord,
        },
    },
    {
        0x1cb8011, // P3s_B1s_N4c_T4c_U2h_U2h
        {
            VertexQPosition,
            V{D::R16, F::UINT, U::BoneIndices},
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
        },
    },
    {
        0xd84e3026, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c_VC4c
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            V{D::R8G8B8A8, F::UNORM, U::BoneWeights},
            VertexBoneIndices,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            VertexTangent,
            VertexColor,
        },
    },
    {
        0xd84e3027, // P3s_W1s_N4c_W4c_B8c_U2h_W2h_T4c_VC4c
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormalSigned,
            V{D::R8G8B8A8, F::UNORM, U::BoneWeights},
            VertexBoneIndices,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            VertexTangentSigned,
            VertexColor,
        },
    },
    {
        0xa8fab009, // P3s_unk1s_N4c_B4c_U2s_T4c
        {
            VertexQPosition,
            V{D::R16, F::UINT, U::Undefined},
            VertexNormalSigned,
            VertexBoneIndices,
            TexCoordPhone,
            VertexTangentSigned,
        },
    },
    {
        0xAE62600B, // P3s_unk1s_N4c_T4c_B4c_W4c_U2h
        {
            VertexQPosition,
            V{D::R16, F::UINT, U::Undefined},
            VertexNormalSigned,
            VertexTangentSigned,
            VertexBoneIndices,
            V{D::R8G8B8A8, F::UNORM, U::BoneWeights},
            TexCoordPhone,
        },
    },
    {
        0x667B1018, // P3s_unk1s_N4c_VC4c_U2h_T4c
        {
            VertexQPosition,
            V{D::R16, F::UINT, U::Undefined},
            VertexNormal,
            VertexColor,
            TexCoord,
            VertexTangent,
        },
    },
    {
        0x667B1019, // P3s_unk1s_N4c_VC4c_U2h_T4c
        {
            VertexQPosition,
            V{D::R16, F::UINT, U::Undefined},
            VertexNormal,
            VertexColor,
            TexCoord,
            VertexTangent,
        },
    },
    {
        0xF606F017,
        {
            VertexPosition,
            VertexNormalSigned,
            V{D::R32G32, F::FLOAT, U::TextureCoordiante},
            V{D::R8G8, F::UINT, U::BoneIndices},
            V{D::R8G8, F::UNORM, U::BoneWeights},
            VertexTangentSigned,
            V{D::R8G8, F::UINT, U::BoneIndices},
            V{D::R8G8, F::UNORM, U::BoneWeights},
        },
    },
    {
        0x01B36016,
        {
            VertexPosition,
            VertexNormalSigned,
            V{D::R32G32, F::FLOAT, U::TextureCoordiante},
            V{D::R8G8, F::UINT, U::BoneIndices},
            V{D::R8G8, F::UNORM, U::BoneWeights},
            VertexTangentSigned, // always zero?
            V{D::R8G8, F::UINT, U::BoneIndices},
            V{D::R8G8, F::UNORM, U::BoneWeights},
        },
    },
    {
        0xD6784014,
        {
            VertexPosition,
            VertexNormalSigned,
            V{D::R32G32, F::FLOAT, U::TextureCoordiante},
            V{D::R8G8, F::UINT, U::BoneIndices},
            V{D::R8G8, F::UNORM, U::BoneWeights},
        },
    },
    {
        0x82917009,
        {
            VertexPosition,
            VertexNormalSigned,
            V{D::R32G32, F::FLOAT, U::TextureCoordiante},
        },
    },
    {
        0x59DC400B,
        {
            VertexPosition,
            VertexNormalSigned,
            V{D::R32G32, F::FLOAT, U::TextureCoordiante},
            VertexTangentSigned,
        },
    },
    {
        0x43FB3015,
        {
            VertexPosition,
            VertexNormalSigned,
            V{D::R32G32, F::FLOAT, U::TextureCoordiante},
            V{D::R8G8, F::UINT, U::BoneIndices},
            V{D::R8G8, F::UNORM, U::BoneWeights},
            VertexTangentSigned,
        },
    },
    {
        0x7BA7401B,
        {
            VertexPosition,
            VertexNormalSigned,
            V{D::R32G32, F::FLOAT, U::TextureCoordiante},
            V{D::R8G8, F::UINT, U::BoneIndices},
            V{D::R8G8, F::UNORM, U::BoneWeights},
            VertexColor,
        },
    },
    {
        0xAE252019,
        {
            VertexPosition,
            VertexNormalSigned,
            V{D::R32G32, F::FLOAT, U::TextureCoordiante},
            V{D::R8G8, F::UINT, U::BoneIndices},
            V{D::R8G8, F::UNORM, U::BoneWeights},
            VertexTangentSigned,
            V{D::R8G8, F::UINT, U::BoneIndices},
            V{D::R8G8, F::UNORM, U::BoneWeights},
            V{D::R8G8, F::UINT, U::BoneIndices},
            V{D::R8G8, F::UNORM, U::BoneWeights},
            V{D::R8G8, F::UINT, U::BoneIndices},
            V{D::R8G8, F::UNORM, U::BoneWeights},
        },
    },
    {
        0x3D62B012,
        {
            VertexPosition,
            VertexNormalSigned,
            V{D::R32G32, F::FLOAT, U::TextureCoordiante},
            VertexColor,
            VertexTangentSigned, // always zero?
            V{D::R32G32, F::FLOAT, U::TextureCoordiante},
        },
    },
    {
        0x6180300A,
        {
            VertexPosition,
            VertexNormalSigned,
            V{D::R32G32, F::FLOAT, U::TextureCoordiante},
            VertexColor,
            VertexTangentSigned,
        },
    },
    {
        0x3F78800C,
        {
            VertexPosition,
            VertexNormalSigned,
            V{D::R32G32, F::FLOAT, U::TextureCoordiante},
            VertexColor,
            VertexTangentSigned,
        },
    },
    {
        0xEDF6F03C,
        {
            VertexPosition,
            VertexNormal,
            VertexTangent,
            VertexBoneIndices,
            V{D::R8G8B8A8, F::UNORM, U::BoneWeights},
            TexCoord,
        },
    },
    {
        0x3339D03A,
        {
            VertexPosition,
            VertexNormal3,
            V{D::R8, F::UINT, U::BoneIndices},
            TexCoord,
        },
    },
    {
        0xFD35504D,
        {
            VertexPosition,
            VertexNormal,
            Packed(VertexColor),
            TexCoord,
            TexCoord,
        },
    },
    {
        0x682EF04C,
        {
            VertexPosition,
            VertexNormal,
            Packed(VertexColor),
            TexCoord,
        },
    },
    {
        0x707FB01C,
        {
            VertexQPosition,
            V{D::R16, F::UINT, U::Undefined}, // bone id?
            VertexNormal,
            VertexTangent,
            TexCoord,
            V{D::R32, F::UINT, U::Undefined},
            V{D::R32, F::UINT, U::Undefined},
            TexCoord,
            VertexColor,
        },
    },
    {
        0xCC510026,
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            VertexBoneIndices,
            TexCoord,
            V{D::R16G16, F::FLOAT, U::BoneWeights},
            V{D::R32, F::UINT, U::Undefined},
            V{D::R32, F::UINT, U::Undefined},
            TexCoord,
            V{D::R32, F::UINT, U::Undefined},
        },
    },
    {
        0x1B9A2021,
        {
            VertexQPosition,
            V{D::R16, F::NORM, U::BoneWeights},
            VertexNormal,
            VertexTangent,
            V{D::R16G16, F::FLOAT, U::BoneIndices},
            TexCoord,
            V{D::R32, F::UINT, U::Undefined},
            V{D::R32, F::UINT, U::Undefined},
            TexCoord,
            V{D::R32, F::UINT, U::Undefined},
        },
    },
    {
        0xC9CFC012, // P3s_B1s_N4c_T4c_U2h_U2h
        {
            VertexQPosition,
            V{D::R16, F::UINT, U::BoneIndices},
            VertexNormal,
            VertexTangent,
            TexCoord,
            TexCoord,
        },
    },
};

static const std::set<uint32> edgeModels{
    0xdb7da014,
    0xdb7da013,
    0xdb7da00d,
    // P3s_unk1s_B4c_W4c_N4c
    0x0CB68015,
    0x0CB68014,
    0x0CB6800e,
    // P3s_B1s_N4c
    0xB0983013,
    // P3s_unk1s_B8c_W8c_N4c
    0xA320C015,
    0xA320C016,
    0xA320C00F,
    0xB0983014,
    0xB0983012,
    0xB098300C,
};

static const auto makeV2 = [](auto &self, revil::MODImpl &main, auto &&fd) {
  revil::MODPrimitive retval;
  uint8 visibleLOD_ = self.data0.template Get<MODMeshXC5::VisibleLOD>();
  const auto visibleLOD = reinterpret_cast<es::Flags<uint8> &>(visibleLOD_);
  retval.lod1 = visibleLOD[0];
  retval.lod2 = visibleLOD[1];
  retval.lod3 = visibleLOD[2];
  retval.materialIndex = self.data0.template Get<MODMeshXC5::MaterialIndex>();
  const MODMeshXC5::PrimitiveType_e primitiveType = MODMeshXC5::PrimitiveType_e(
      self.data1.template Get<
          typename std::decay_t<decltype(self)>::PrimitiveType>());

  retval.triStrips = primitiveType == MODMeshXC5::PrimitiveType_e::Strips;
  retval.indexIndex = main.indices.size();
  retval.vertexIndex = main.vertices.size();
  retval.meshId = self.meshIndex;
  retval.groupId = self.data0.template Get<MODMeshXC5::GroupID>();
  const size_t vertexStride =
      self.data1.template Get<MODMeshXC5::VertexBufferStride>();

  char *mainBuffer =
      main.vertexBuffer.data() + (self.vertexStart * vertexStride) +
      self.vertexStreamOffset + (self.indexValueOffset * vertexStride);

  auto foundFormat = formats.find(self.vertexFormat);

  if (!es::IsEnd(formats, foundFormat)) {
    if (foundFormat->second.stride != vertexStride) {
      foundFormat = fallbackFormats.find(self.vertexFormat);
      if (es::IsEnd(fallbackFormats, foundFormat)) {
        throw std::runtime_error("Cannot find fallback vertex format: " +
                                 std::to_string(self.vertexFormat));
      }
    }

    MODVertexSpan tmpl;
    tmpl.buffer = mainBuffer;
    tmpl.attrs = foundFormat->second.attrs;
    tmpl.stride = vertexStride;
    tmpl.numVertices = self.numVertices;
    fd(tmpl);
    for (auto &a : tmpl.attrs) {
      a.offset = -1;
    }

    main.vertices.emplace_back(std::move(tmpl));
  } else {
    main.vertices.emplace_back();

    if (!edgeModels.contains(self.vertexFormat)) {
      throw std::runtime_error("Unregistered vertex format: " +
                               std::to_string(self.vertexFormat));
    }
  }

  uint16 *indexBuffer = main.indexBuffer.data() + self.indexStart;

  MODIndexSpan idArray(indexBuffer, self.numIndices);

  for (auto &idx : idArray) {
    if (idx != 0xffff) {
      idx -= self.vertexStart;
    }
  }

  main.indices.emplace_back(idArray);

  return retval;
};

revil::MODPrimitive MODMeshXD2::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXD2> &>(main_);
  return makeV2(*this, main, [&](MODVertexSpan &d) {
    for (auto &a : d.attrs) {
      if (a.usage == AttributeType::BoneIndices && skinBoneBegin) {
        auto mcdx = std::make_unique<AttributeAdd>();
        mcdx->add = skinBoneBegin;
      }
    }
  });
}

revil::MODPrimitive MODMeshXD2::ReflectBE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXD2> &>(main_);
  return makeV2(*this, main, [&](MODVertexSpan &d) {
    for (auto &a : d.attrs) {
      if (a.usage == AttributeType::BoneIndices &&
          skinBoneBegin < main.bones.size()) {
        auto mcdx = std::make_unique<AttributeAdd>();
        mcdx->add = skinBoneBegin;
      }
    }

    swapBuffers(d);
  });
}

revil::MODPrimitive MODMeshXD3PS4::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXD2> &>(main_);
  return makeV2(*this, main, [&](MODVertexSpan &d) {
    for (auto &a : d.attrs) {
      if (a.usage == AttributeType::BoneIndices && skinBoneBegin) {
        auto mcdx = std::make_unique<AttributeAdd>();
        mcdx->add = skinBoneBegin;
      } else if (a.usage == AttributeType::Normal) {
        a.format = VertexNormalSigned.format;
        a.customCodec = nullptr;
      } else if (a.usage == AttributeType::Tangent) {
        a.format = VertexTangentSigned.format;
        a.customCodec = nullptr;
      }
    }
  });
}

revil::MODPrimitive MODMeshXD3::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXD2> &>(main_);
  return makeV2(*this, main, [&](MODVertexSpan &d) {
    for (auto &a : d.attrs) {
      if (a.usage == AttributeType::BoneIndices && skinBoneBegin) {
        auto mcdx = std::make_unique<AttributeAdd>();
        mcdx->add = skinBoneBegin;
      }
    }
  });
}

revil::MODPrimitive MODMeshXC5::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXC5> &>(main_);
  return makeV2(*this, main, [&](MODVertexSpan &) {});
}

revil::MODPrimitive MODMeshXC5::ReflectBE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXC5> &>(main_);
  return makeV2(*this, main, [&](MODVertexSpan &d) { swapBuffers(d); });
}

revil::MODPrimitive MODMeshX06::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXD2> &>(main_);
  auto retval = makeV2(*this, main, [&](MODVertexSpan &) {});
  retval.skinIndex = skinBoneBegin;

  /*auto idxArray = main.Indices()->At(retval.indexIndex);
  std::span<const uint16> indices(
      reinterpret_cast<const uint16 *>(idxArray->RawIndexBuffer()), numIndices);

  bool removeTS = true;

  for (auto d : vtArray.descs) {
    switch (d->Usage()) {
    case AttributeType::Normal: {
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
    std::remove_if(vtArray.descs.storage.begin(), vtArray.descs.storage.end(,},
                   [](MODVertexDescriptor &d) {
                     return d.usage == MODVertexDescriptor::Usage_e::Normal;
                   });
    std::remove_if(vtArray.descs.storage.begin(), vtArray.descs.storage.end(,},
                   [](MODVertexDescriptor &d) {
                     return d.usage == MODVertexDescriptor::Usage_e::Tangent;
                   });
  }*/

  return retval;
}

revil::MODPrimitive MODMeshXE5::ReflectBE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXD2> &>(main_);
  auto retval = makeV2(*this, main, [&](MODVertexSpan &d) { swapBuffers(d); });
  retval.skinIndex = numEnvelopes;

  return retval;
}

revil::MODPrimitive MODMeshXE5::ReflectLE(revil::MODImpl &main_) {
  auto &main = static_cast<MODInner<MODTraitsXD2> &>(main_);
  auto retval = makeV2(*this, main, [&](MODVertexSpan &) {});
  retval.skinIndex = numEnvelopes;

  return retval;
}

MOD::MOD() {}
MOD::MOD(MOD &&) = default;
MOD::~MOD() = default;

namespace revil {
std::span<const MODVertexSpan> MOD::Vertices() const { return pi->vertices; }
std::span<const MODIndexSpan> MOD::Indices() const { return pi->indices; }
std::span<const revil::MODPrimitive> MOD::Primitives() const {
  return pi->primitives;
}
std::span<const MODSkinJoints> MOD::SkinJoints() const { return pi->skins; }
std::span<const MODMaterial> MOD::Materials() const { return pi->materialRefs; }
std::span<const es::Matrix44> MOD::InverseBinds() const {
  return pi->transforms;
}
std::span<const es::Matrix44> MOD::Transforms() const { return pi->refPoses; }

std::span<const MODBone> MOD::Bones() const { return pi->simpleBones; }
std::span<const MODGroup> MOD::Groups() const { return pi->groups; }
std::span<const MODEnvelope> MOD::Envelopes() const { return pi->envelopes; }
const MODMetaData &MOD::Metadata() const { return pi->Metadata(); }
} // namespace revil
