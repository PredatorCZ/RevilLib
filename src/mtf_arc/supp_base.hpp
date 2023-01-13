/*  Revil Format Library
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
#include "revil/platform.hpp"
#include <map>
#include <stdexcept>

using namespace revil;

struct TitleSupports {
  std::map<Platform, const TitleSupport *> data;

  void Assign(Platform platform, const TitleSupport &storage) {
    data.emplace(platform, &storage);
  }

  template <class... type>
  void Assign(Platform platform, const TitleSupport &storage, type &&...types) {
    Assign(platform, storage);
    Assign(std::forward<type>(types)...);
  }

  template <class... type> TitleSupports(type &&...types) {
    Assign(std::forward<type>(types)...);
  }

  auto Get(Platform platform) const { return data.at(platform); }
};

static constexpr ArcSupport ARC_PS3_GENERIC{8, 14, true};
static constexpr ArcSupport ARC_WINPC_GENERIC{};
static constexpr ArcSupport ARC_N3DS_GENERIC{0x11};
