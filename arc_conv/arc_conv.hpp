/*  ARCConvert
    Copyright(C) 2021 Lukas Cone

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

static thread_local std::stringstream titleStream;
static thread_local size_t cIndent = 1;

void AddTitle(es::string_view titleName) {
  titleStream << std::setfill('\t') << std::setw(cIndent + 1) << '\t'
              << titleName;
  PlatformFlags flags = GetPlatformSupport(titleName);
  bool added = false;
  auto ref = GetReflectedEnum<Platform>();

  for (size_t f = 1; f < ref->numMembers; f++) {
    if (flags[Platform(f)]) {
      if (!added) {
        titleStream << " (" << ref->names[f];
        added = true;
        continue;
      }
      titleStream << ", " << ref->names[f];
    }
  }

  if (added) {
    titleStream << ')';
  }

  titleStream << std::endl;
}

void AppAdditionalHelp(std::ostream &str, size_t indent) {
  titleStream.str("");
  cIndent = indent;
  str << std::setfill('\t') << std::setw(indent) << '\t'
      << "Valid titles: title ( supported platforms )" << std::endl;
  revil::GetTitles(AddTitle);
  str << titleStream.str();
}

void AppInitModule() { RegisterReflectedType<Platform>(); }
