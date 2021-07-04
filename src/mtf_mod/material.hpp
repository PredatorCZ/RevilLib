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
#include "datas/vectors_simd.hpp"
#include "datas/bitfield.hpp"

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

  uint32 bitField00; // PSH
  VSHData vshData;
  uint32 internal[2]; // assigned at runtime
  uint32 shaders[2];  // assigned at runtime
  uint32 textureIDs[9];

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

  PSHData pshData;
  VSHData vshData;
  uint32 null00;
  uint32 baseTextureIndex;
  uint32 normalTextureIndex;
  uint32 maskTextureIndex;
  uint32 lightTextureIndex;
  uint32 shadowTextureIndex;
  uint32 additionalTextureIndex; // Emissive or alpha mask
  uint32 cubeMapTextureIndex;
  uint32 detailTextureIndex;
  uint32 AOTextureIndex;

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

struct MODMaterialX170 {
  uint32 bitField00;
  uint32 bitField01;
  uint32 internal[2]; // assigned at runtime
  uint64 shaders[2];  // assigned at runtime
  uint64 textureIDs[9];

  float transparency;
  uint32 unk02;
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
};
