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
#include "bones.hpp"
#include "common.hpp"
#include "material.hpp"
#include "mesh.hpp"

struct MODTraitsX70 {
  static constexpr size_t numSkinRemaps = 1;
  static constexpr size_t numRemaps = 1;
  static constexpr size_t pathSize = 64;
  using primitive = MODPrimitiveProxyV1;
  using bone = MODBoneV1_5;
  using material = MODMaterialX70;
  using mesh = MODMeshX70;
  using metadata = MODMetaDataV1;
};

struct MODTraitsX170 : MODTraitsX70 {
  using material = MODMaterialX170;
};

struct MODTraitsX99LE {
  static constexpr size_t numSkinRemaps = 32;
  static constexpr size_t numRemaps = 0x100;
  static constexpr size_t pathSize = 64;
  using primitive = MODPrimitiveProxyV1;
  using bone = MODBoneV1_5;
  using material = MODMaterialX70;
  using mesh = MODMeshX99;
  using metadata = MODMetaDataV1;
};

struct MODTraitsX99BE : MODTraitsX99LE {
  static constexpr size_t numSkinRemaps = 64;
};

struct MODTraitsXC5 {
  static constexpr size_t numSkinRemaps = 1;
  static constexpr size_t numRemaps = 0x100;
  static constexpr size_t pathSize = 64;
  using primitive = MODPrimitiveProxy;
  using bone = MODBoneV1_5;
  using material = MODMaterialXC5;
  using mesh = MODMeshXC5;
  using metadata = MODMetaDataV1;
};

struct MODTraitsXD3 {
  static constexpr size_t numSkinRemaps = 1;
  static constexpr size_t numRemaps = 0x100;
  static constexpr size_t pathSize = 1;
  using primitive = MODPrimitiveProxy;
  using bone = MODBoneV1_5;
  using material = MODMaterialHash;
  using mesh = MODMeshXD2;
  using metadata = MODMetaDataV2;
};

struct MODTraitsXD2 {
  static constexpr size_t numSkinRemaps = 1;
  static constexpr size_t numRemaps = 0x100;
  static constexpr size_t pathSize = 1;
  using primitive = MODPrimitiveProxy;
  using bone = MODBoneV1_5;
  using material = MODMaterialName;
  using mesh = MODMeshXD2;
  using metadata = MODMetaDataV2;
};

struct MODTraitsXD3PS4 : MODTraitsXD2 {
  using mesh = MODMeshXD3PS4;
};

struct MODTraitsX06 {
  static constexpr size_t numSkinRemaps = 32;
  static constexpr size_t numRemaps = 0x100;
  static constexpr size_t pathSize = 1;
  using primitive = MODPrimitiveProxy;
  using bone = MODBoneV1_5;
  using material = MODMaterialName;
  using mesh = MODMeshX06;
  using metadata = MODMetaDataV1;
};
