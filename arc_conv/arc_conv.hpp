/*  ARCConvert
    Copyright(C) 2021-2022 Lukas Cone

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
#include "arc.hpp"
#include "re_common.hpp"
#include "revil/hashreg.hpp"
#include "zlib.h"
#include <iomanip>
#include <sstream>

void AppAdditionalHelp(AppHelpContext *ctx, size_t indent) {
  auto &str = ctx->GetStream("titles");
  str << std::setfill('\t') << std::setw(indent) << '\t'
      << "Valid titles: title ( supported platforms )" << std::endl;
  revil::GetTitles([&](std::string_view titleName) {
    str << std::setw(indent + 1) << '\t' << titleName;
    PlatformFlags flags = GetPlatformSupport(titleName);
    bool added = false;
    auto ref = GetReflectedEnum<Platform>();

    for (size_t f = 1; f < ref->numMembers; f++) {
      if (flags[Platform(f)]) {
        if (!added) {
          str << " (" << ref->names[f];
          added = true;
          continue;
        }
        str << ", " << ref->names[f];
      }
    }

    if (added) {
      str << ')';
    }

    str << std::endl;
  });
}
