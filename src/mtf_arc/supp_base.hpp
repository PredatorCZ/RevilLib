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

using namespace revil;

struct TitleSupports {
  static constexpr size_t NUMSLOTS = 10;
  const TitleSupport *data[NUMSLOTS]{};

  static size_t Index(Platform platform) {
    return static_cast<size_t>(platform);
  }

  void Assign(Platform platform, const TitleSupport &storage) {
    data[Index(platform)] = &storage;
  }

  template <class... type>
  void Assign(Platform platform, const TitleSupport &storage,
              type... types) {
    Assign(platform, storage);
    Assign(types...);
  }

  template <class... type>
  TitleSupports(type... types) {
    Assign(types...);
  }

  auto Get(Platform platform) const {
    const size_t index = Index(platform);
    if (index >= NUMSLOTS) {
      throw std::out_of_range("Platform id out of range.");
    }
    auto item = data[index];

    if (!item) {
      throw std::logic_error("Invalid platform.");
    }
    return item;
  }
};

static constexpr ArcSupport ARC_PS3_GENERIC{8, 14, true};
static constexpr ArcSupport ARC_WINPC_GENERIC{};
static constexpr ArcSupport ARC_N3DS_GENERIC{0x11};
