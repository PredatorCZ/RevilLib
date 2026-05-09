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
#include "revil/mod.hpp"
#include "spike/type/bitfield.hpp"
#include "spike/type/flags.hpp"
#include "spike/type/vectors_simd.hpp"

namespace revil {
class MODImpl;
}

struct MODMeshSkinInfo {
  uint8 numEnvelopes;
  uint8 boneRemapIndex;
  uint16 unk;
  void Swap();
};

struct MODMeshX99 {
  uint16 groupIndex;
  uint16 materialIndex;
  bool visible;
  es::Flags<uint8> visibleLOD; // only 3 LODs
  uint8 alphaType;
  uint8 numWeights;
  uint8 buffer0Stride;
  uint8 buffer1Stride;
  uint8 connective;
  uint8 shape;
  uint16 numVertices;
  uint16 endIndex;
  uint32 vertexStart;
  uint32 vertexStreamOffset;
  uint32 vertexStream2Offset;
  uint32 indexStart;
  uint32 numIndices;
  uint32 indexValueOffset;
  uint8 unk050[2];
  uint16 startIndex;
  MODMeshSkinInfo skinInfo;
  uint32 firstEnvelope; // assigned at runtime

  revil::MODPrimitive ReflectLE(revil::MODImpl &);
  revil::MODPrimitive ReflectBE(revil::MODImpl &);
};

struct MODMeshX70 {
  uint16 groupIndex;
  uint16 materialIndex;
  bool visible;
  es::Flags<uint8> visibleLOD; // only 3 LODs
  uint8 alphaType;
  uint8 numWeights;
  uint8 buffer0Stride;
  uint8 buffer1Stride;
  uint8 connective;
  uint8 shape;
  uint32 numVertices;
  uint32 vertexStart;
  uint32 vertexStreamOffset;
  uint32 vertexStream2Offset;
  uint32 indexStart;
  uint32 numIndices;
  uint32 indexValueOffset;
  Vector4A16 bboxMin;
  Vector4A16 bboxMax;

  revil::MODPrimitive ReflectBE(revil::MODImpl &);
  revil::MODPrimitive ReflectLE(revil::MODImpl &);
};

struct MODMeshXC5 {
  using GroupID = BitMemberDecl<0, 12>;
  using MaterialIndex = BitMemberDecl<1, 12>;
  using VisibleLOD = BitMemberDecl<2, 8>;
  using BitField00 = BitFieldType<uint32, GroupID, MaterialIndex, VisibleLOD>;

  using Visible = BitMemberDecl<0, 1>;
  using Shape = BitMemberDecl<1, 1>;
  using Sort = BitMemberDecl<2, 1>;
  using NumWeights = BitMemberDecl<3, 5>;
  using AlphaType = BitMemberDecl<4, 8>;
  using VertexBufferStride = BitMemberDecl<5, 8>;
  using PrimitiveType = BitMemberDecl<6, 8>;
  using BitField01 = BitFieldType<uint32, Visible, Shape, Sort, NumWeights,
                                  AlphaType, VertexBufferStride, PrimitiveType>;

  enum class PrimitiveType_e : uint8 {
    Points,
    Lines,
    LineStrips,
    Triangles,
    Strips,
  };

  uint16 drawMode;
  uint16 numVertices;
  BitField00 data0;
  BitField01 data1;
  uint32 vertexStart;
  uint32 vertexStreamOffset;
  uint32 vertexFormat;
  uint32 indexStart;
  uint32 numIndices;
  uint32 indexValueOffset;
  uint16 numEnvelopes;
  uint16 meshIndex;
  uint16 minVertex;
  uint16 maxVertex;
  uint32 boundaryInfo; // rt ptr

  revil::MODPrimitive ReflectLE(revil::MODImpl &);
  revil::MODPrimitive ReflectBE(revil::MODImpl &);
};

struct MODMeshXD2 {
  using PrimitiveType = BitMemberDecl<6, 6>;
  using BinormalFlip = BitMemberDecl<7, 1>;
  using Bridge = BitMemberDecl<8, 1>;
  using BitField01 =
      BitFieldType<uint32, MODMeshXC5::Visible, MODMeshXC5::Shape,
                   MODMeshXC5::Sort, MODMeshXC5::NumWeights,
                   MODMeshXC5::AlphaType, MODMeshXC5::VertexBufferStride,
                   PrimitiveType, BinormalFlip, Bridge>;
  uint16 drawMode;
  uint16 numVertices;
  MODMeshXC5::BitField00 data0;
  BitField01 data1;
  uint32 vertexStart;
  uint32 vertexStreamOffset;
  uint32 vertexFormat;
  uint32 indexStart;
  uint32 numIndices;
  uint32 indexValueOffset;
  uint8 skinBoneBegin;
  uint8 numEnvelopes;
  uint16 meshIndex;
  uint16 minVertex;
  uint16 maxVertex;
  uint32 boundaryInfo; // rt ptr

  revil::MODPrimitive ReflectLE(revil::MODImpl &);
  revil::MODPrimitive ReflectBE(revil::MODImpl &);
};

struct MODMeshXD3 : MODMeshXD2 {
  revil::MODPrimitive ReflectLE(revil::MODImpl &);
  revil::MODPrimitive ReflectBE(revil::MODImpl &) { return {}; }
  void NoSwap();
};

struct MODMeshXD3PS4 : MODMeshXD2 {
  revil::MODPrimitive ReflectLE(revil::MODImpl &);
  revil::MODPrimitive ReflectBE(revil::MODImpl &) { return {}; }
  void NoSwap();
};

struct MODMeshX06 : MODMeshXD2 {
  revil::MODPrimitive ReflectLE(revil::MODImpl &);
  revil::MODPrimitive ReflectBE(revil::MODImpl &) { return {}; }
  void NoSwap();
};

struct MODMeshXE5 : MODMeshXD2 {
  revil::MODPrimitive ReflectLE(revil::MODImpl &);
  revil::MODPrimitive ReflectBE(revil::MODImpl &);
};
