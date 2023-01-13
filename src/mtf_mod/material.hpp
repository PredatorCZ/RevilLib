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
#include "datas/bitfield.hpp"
#include "datas/vectors_simd.hpp"

struct MODMaterialX70 {
  using SkinType = BitMemberDecl<0, 4>;
  using Unk00 = BitMemberDecl<1, 2>;
  using LightningType = BitMemberDecl<2, 4>;
  using NormalMapType = BitMemberDecl<3, 4>;
  using SpecularType = BitMemberDecl<4, 4>;
  using LightMapType = BitMemberDecl<5, 4>;
  using MultiTextureType = BitMemberDecl<6, 4>;
  using Unk01 = BitMemberDecl<7, 4>;
  using Unk02 = BitMemberDecl<8, 2>;
  using VSHData =
      BitFieldType<uint32, SkinType, Unk00, LightningType, NormalMapType,
                   SpecularType, LightMapType, MultiTextureType, Unk01, Unk02>;

  static constexpr size_t Version() { return 1; }

  uint32 pshData;
  VSHData vshData;
  uint32 internal[2]; // assigned at runtime
  uint32 shaders[2];  // assigned at runtime
  int32 baseTextureIndex;
  int32 normalTextureIndex;
  int32 maskTextureIndex;
  int32 lightTextureIndex;
  int32 shadowTextureIndex;
  int32 additionalTextureIndex; // Emissive or alpha mask
  int32 cubeMapTextureIndex;
  int32 detailTextureIndex;
  int32 AOTextureIndex;

  float transparency;
  float fresnelFactor;
  float fresnelBias;
  float specularPower;
  float envMapPower;
  Vector4A16 lightMapScale;
  float detailFactor;
  float detailWrap;
  float envMapBias;
  float normalBias;
  Vector4A16 transmit;
  Vector4A16 paralax;

  uint32 hash;
  uint8 unk;

  std::string Name() const;
};

struct MODMaterialX170 {
  static constexpr size_t Version() { return 1; }

  uint32 pshData;
  MODMaterialX70::VSHData vshData;
  uint32 internal[2]; // assigned at runtime
  uint64 shaders[2];  // assigned at runtime
  int64 baseTextureIndex;
  int64 normalTextureIndex;
  int64 maskTextureIndex;
  int64 lightTextureIndex;
  int64 shadowTextureIndex;
  int64 additionalTextureIndex; // Emissive or alpha mask
  int64 cubeMapTextureIndex;
  int64 detailTextureIndex;
  int64 AOTextureIndex;

  float transparency;
  uint32 unk00;
  float fresnelFactor;
  float fresnelBias;
  float specularPower;
  float envMapPower;
  Vector4A16 lightMapScale;
  float detailFactor;
  float detailWrap;
  float envMapBias;
  float normalBias;
  Vector4A16 transmit;
  Vector4A16 paralax;

  uint32 hash;
  uint8 unk01;
};


struct MODMaterialXC5 {
  using SkinType = BitMemberDecl<0, 4>;
  using Unk00 = BitMemberDecl<1, 2>; // null
  using LightningType = BitMemberDecl<2, 4>;
  using NormalMapType = BitMemberDecl<3, 4>;
  using SpecularType = BitMemberDecl<4, 4>;
  using LightMapType = BitMemberDecl<5, 4>;
  using MultiTextureType = BitMemberDecl<6, 4>;
  using Unk01 = BitMemberDecl<7, 6>; // null
  using VSHData =
      BitFieldType<uint32, SkinType, Unk00, LightningType, NormalMapType,
                   SpecularType, LightMapType, MultiTextureType, Unk01>;

  using Unk02 = BitMemberDecl<0, 2>; // 0, 1 = transparent, 2, 3 = opaque
  using Unk03 = BitMemberDecl<1, 4>;
  using Unk04 = BitMemberDecl<2, 2>;
  using Unk05 = BitMemberDecl<3, 2>;
  using Unk06 = BitMemberDecl<4, 1>;
  using Unk07 = BitMemberDecl<5, 3>;
  using Unk08 = BitMemberDecl<6, 8>;
  using Unk09 = BitMemberDecl<7, 5>;
  using Unk10 = BitMemberDecl<8, 4>;
  using Unk11 = BitMemberDecl<9, 1>;

  using PSHData = BitFieldType<uint32, Unk02, Unk03, Unk04, Unk05, Unk06, Unk07,
                               Unk08, Unk09, Unk10, Unk11>;

  static constexpr size_t Version() { return 1; }

  PSHData pshData;
  VSHData vshData;
  uint32 null00;
  int32 baseTextureIndex;
  int32 normalTextureIndex;
  int32 maskTextureIndex;
  int32 lightTextureIndex;
  int32 shadowTextureIndex;
  int32 additionalTextureIndex; // Emissive or alpha mask
  int32 cubeMapTextureIndex;
  int32 detailTextureIndex;
  int32 AOTextureIndex;

  float transparency;
  uint32 unk00[3]; // non essential
  float unk01[2];  // fresnel??
  float specularPower;
  float envMapPower;
  Vector4A16 lightMapScale;
  float detailFactor;
  float detailWrap;
  float envMapBias;
  float normalBias;
  float unk02;
  float null01[3];
  float unk03;
  float unk04;
  uint32 unk05;
  float unk06;
  uint32 unk07;
  uint8 unk08;
};

struct MODMaterialHash {
  uint32 hash;
  static constexpr size_t Version() { return 1; }
  std::string Name() const;
};

struct MODMaterialName {
  char name[0x80];
  static constexpr size_t Version() { return 1; }
  std::string Name() const;
  void NoSwap();
};
