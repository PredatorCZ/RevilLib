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
#include "revil/types.hpp"
#include "spike/type/bitfield.hpp"
#include "spike/util/pugi_fwd.hpp"

struct MODMaterialX70 {
  using SkinType = BitMemberDecl<0, 4>;
  using Reserved2 = BitMemberDecl<1, 2>;
  using LightningType = BitMemberDecl<2, 4>;
  using NormalMapType = BitMemberDecl<3, 4>;
  using SpecularType = BitMemberDecl<4, 4>;
  using LightMapType = BitMemberDecl<5, 4>;
  using MultiTextureType = BitMemberDecl<6, 4>;
  using Reserved = BitMemberDecl<7, 6>;
  using VSHData =
      BitFieldType<uint32, SkinType, Reserved2, LightningType, NormalMapType,
                   SpecularType, LightMapType, MultiTextureType, Reserved>;

  using EnableFOG = BitMemberDecl<0, 1>;
  using ZWrite = BitMemberDecl<1, 1>;
  using Attr = BitMemberDecl<2, 12>;
  using No = BitMemberDecl<3, 8>;
  using EnvmapBias = BitMemberDecl<4, 5>;
  using VType = BitMemberDecl<5, 3>;
  using EnableUVScroll = BitMemberDecl<6, 1>;
  using ZTest = BitMemberDecl<7, 1>;
  using PSHData = BitFieldType<uint32, EnableFOG, ZWrite, Attr, No, EnvmapBias,
                               VType, EnableUVScroll, ZTest>;

  PSHData pshData;
  VSHData vshData;
  uint32 technique;
  uint32 pipeline;       // assigned at runtime
  uint32 vertexDescs[2]; // assigned at runtime
  int32 baseTextureIndex;
  int32 normalTextureIndex;
  int32 maskTextureIndex;
  int32 lightTextureIndex;
  int32 shadowTextureIndex;
  int32 additionalTextureIndex; // Emissive or alpha mask
  int32 cubeMapTextureIndex;
  int32 heightTextureIndex;
  int32 glossTextureIndex;

  float transparency;
  MtVector4 fresnelFactor;
  MtVector4 lightMapScale;
  MtVector4 detailFactor;
  MtVector4 transmit;
  MtVector4 paralax;
  uint32 blendState;
  uint8 alphaRef;

  std::string Name() const;
  void ToXML(pugi::xml_node node) const;
};

struct MODMaterialX170 {
  using SkinType = MODMaterialX70::SkinType;

  MODMaterialX70::PSHData pshData;
  MODMaterialX70::VSHData vshData;
  uint32 technique;
  uint32 pipeline;       // assigned at runtime
  uint64 vertexDescs[2]; // assigned at runtime
  int64 baseTextureIndex;
  int64 normalTextureIndex;
  int64 maskTextureIndex;
  int64 lightTextureIndex;
  int64 shadowTextureIndex;
  int64 additionalTextureIndex; // Emissive or alpha mask
  int64 cubeMapTextureIndex;
  int64 heightTextureIndex;
  int64 glossTextureIndex;

  float transparency;
  MtVector4 fresnelFactor;
  MtVector4 lightMapScale;
  MtVector4 detailFactor;
  MtVector4 transmit;
  MtVector4 paralax;
  uint32 blendState;
  uint8 alphaRef;

  void ToXML(pugi::xml_node node) const;
};

struct MODMaterialXC5 {
  MODMaterialX70::PSHData pshData;
  MODMaterialX70::VSHData vshData;
  uint32 technique;
  int32 baseTextureIndex;
  int32 normalTextureIndex;
  int32 maskTextureIndex;
  int32 lightTextureIndex;
  int32 shadowTextureIndex;
  int32 additionalTextureIndex; // Emissive or alpha mask
  int32 cubeMapTextureIndex;
  int32 heightTextureIndex;
  int32 glossTextureIndex;

  float transparency;
  MtVector4 fresnelFactor;
  MtVector4 lightMapScale;
  MtVector4 detailFactor;
  MtVector4 transmit;
  MtVector4 paralax;
  uint32 blendState;
  uint8 alphaRef;

  void ToXML(pugi::xml_node node) const;
};

struct MODMaterialHash {
  uint32 hash;
  std::string Name() const;
  void ToXML(pugi::xml_node node) const;
};

struct MODMaterialName {
  char name[0x80];
  std::string Name() const;
  void ToXML(pugi::xml_node node) const;
  void NoSwap();
};

struct MODMaterialX21 {
  std::string name;
  std::string Name() const { return name; }
  void ToXML(pugi::xml_node node) const;
};
