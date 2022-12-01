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
#include "datas/supercore.hpp"

namespace revil {
enum class Platform {
  Auto,
  WinPC,
  PS3,
  X360,
  N3DS,
  CAFE,
  NSW,
  PS4,
  Android,
  IOS,
};

struct ArcSupport {
  uint16 version = 7;
  uint16 windowSize = 15;
  bool allowRaw = false;
  bool xmemOnly = false;
  bool encrypted = false;
  bool extendedFilePath = false;
};

struct ModSupport {
  uint16 version = 0;
  bool x64 = false;
};

struct TexSupport {
  uint16 version = 0;
  bool x64 = false;
};

struct LmtSupport {
  uint16 version = 0;
  bool x64 = false;
};

struct TitleSupport {
  ArcSupport arc;
  ModSupport mod;
  TexSupport tex;
  LmtSupport lmt;
};

} // namespace revil
