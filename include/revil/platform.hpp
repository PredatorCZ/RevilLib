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
#include "spike/util/supercore.hpp"
#include <string_view>

namespace revil {
enum class Platform {
  Auto,
  Win32,
  N3DS,
  PS3 = 0100 | ((N3DS & 077) + 1),
  X360,
  CAFE,
  NSW = 0200 | ((CAFE & 077) + 1),
  PS4,
  Android,
  IOS,
  Win64,
};

struct PlatformInfo {
  bool x64 = false;
  bool bigEndian = false;

  PlatformInfo(Platform p)
      : x64(uint32(p) & 0200), bigEndian(uint32(p) & 0100) {}
};

template <class C> struct SmallArray {
  uint32 size : 8;
  int32 offset : 24;

  const C *operator->() const {
    return reinterpret_cast<const C *>(
        (offset != 0) * reinterpret_cast<intptr_t>(this) + offset);
  }

  const C *begin() const { return operator->(); }
  const C *end() const { return begin() + size; }
};

struct StringEntry : SmallArray<char> {
  operator std::string_view() const { return {operator->(), size}; }
};

static_assert(sizeof(StringEntry) == 4);

enum DbArcFlags {
  DbArc_AllowRaw = 1,
  DbArc_ExtendedPath = 2,
  DbArc_XMemCompress = 4,
  DbArc_Version1 = 8,
};

struct DbArcSupport {
  uint8 version;
  uint8 windowSize;
  uint8 flags;
  StringEntry key;
};

struct TitleSupport {
  DbArcSupport arc;
  uint16 modVersion;
  uint16 lmtVersion;
  uint16 texVersion;
  uint16 xfsVersion;
};

} // namespace revil
