/*  Revil Format Library
    Copyright(C) 2020 Lukas Cone

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
#include <string_view>
#include "supp_base.hpp"
#include <map>

using MtExtensionsStorage = std::multimap<std::string_view, uint32>;
using MtExtFixupStorage = std::map<uint32, std::string_view>;

struct MtExtensions {
  static constexpr size_t NUMSLOTS = 10;
  const MtExtensionsStorage *data[NUMSLOTS]{};
  const MtExtFixupStorage *fixups = nullptr;
  const TitleSupports *support = nullptr;

  static size_t Index(Platform platform) {
    return static_cast<size_t>(platform);
  }

  void Assign(Platform platform, const MtExtensionsStorage &storage) {
    data[Index(platform)] = &storage;
  }

  template <class... type>
  void Assign(Platform platform, const MtExtensionsStorage &storage,
              type&&... types) {
    Assign(platform, storage);
    Assign(std::forward<type>(types)...);
  }

  void Assign(Platform platform) { data[Index(platform)] = data[0]; }

  void Assign(const MtExtFixupStorage &storage) { fixups = &storage; }

  template <class... type>
  void Assign(const MtExtFixupStorage &storage, type&&... types) {
    Assign(storage);
    Assign(std::forward<type>(types)...);
  }

  void Assign(const TitleSupports &storage) { support = &storage; }

  template <class... type>
  void Assign(const TitleSupports &storage, type&&... types) {
    Assign(storage);
    Assign(std::forward<type>(types)...);
  }

  template <class... type>
  MtExtensions(const MtExtensionsStorage &base, type&&... types) : data{&base} {
    Assign(std::forward<type>(types)...);
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

  auto Base() const { return data[0]; }

  uint32 GetHash(std::string_view extension, Platform platform) const {
    auto found = Base()->find(extension);

    if (!es::IsEnd(*Base(), found)) {
      return found->second;
    }

    if (platform != Platform::Auto) {
      auto store = Get(platform);
      auto found_ = store->find(extension);

      if (!es::IsEnd(*store, found_)) {
        return found_->second;
      }
    }

    return 0;
  }
};
