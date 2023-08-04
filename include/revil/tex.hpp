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
#include "spike/io/bincore_fwd.hpp"
#include "spike/type/vectors_simd.hpp"
#include "spike/format/DDS.hpp"
#include "platform.hpp"
#include "settings.hpp"
#include <string>

namespace revil {

struct Tex2DdsSettings {
  bool convertIntoLegacy = false;
  bool convertIntoLegacyNonCannon = false;
  bool noMips = false;
  Platform platformOverride = Platform::Auto;
};

struct RE_EXTERN TEX {
  DDS asDDS{};
  Vector4A16 color;
  std::string buffer;
  DDS::Mips mips;

  void Load(BinReaderRef_e rd, Platform platform = Platform::Auto);
  void SaveAsDDS(BinWritterRef wr, Tex2DdsSettings settings);
};
} // namespace revil
