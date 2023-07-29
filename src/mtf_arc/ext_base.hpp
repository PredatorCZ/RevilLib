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
#include "revil/settings.hpp"
#include "supp_base.hpp"
#include <map>
#include <string_view>

// Convert extension to class hash
using MtExtensionsStorage = std::multimap<std::string_view, uint32>;

// Priority extension lookup for current title
using MtExtFixupStorage = std::map<uint32, std::string_view>;

struct MtExtensions {
  std::map<Platform, const MtExtensionsStorage *> data;
  const MtExtFixupStorage *fixups = nullptr;
  const TitleSupports *support = nullptr;

  void Assign(Platform platform, const MtExtensionsStorage &storage) {
    data.emplace(platform, &storage);
  }

  template <class... type>
  void Assign(Platform platform, const MtExtensionsStorage &storage,
              type &&...types) {
    Assign(platform, storage);
    Assign(std::forward<type>(types)...);
  }

  void Assign(Platform platform) {
    data.emplace(platform, data.at(Platform::Auto));
  }

  void Assign(const MtExtFixupStorage &storage) { fixups = &storage; }

  template <class... type>
  void Assign(const MtExtFixupStorage &storage, type &&...types) {
    Assign(storage);
    Assign(std::forward<type>(types)...);
  }

  void Assign(const TitleSupports &storage) { support = &storage; }

  template <class... type>
  void Assign(const TitleSupports &storage, type &&...types) {
    Assign(storage);
    Assign(std::forward<type>(types)...);
  }

  template <class... type>
  MtExtensions(const MtExtensionsStorage &base, type &&...types)
      : data{{Platform::Auto, &base}} {
    Assign(std::forward<type>(types)...);
  }

  auto Get(Platform platform) const { return data.at(platform); }

  auto Base() const { return data.at(Platform::Auto); }

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

namespace revil {
const MtExtensions RE_EXTERN *GetTitleRegistry(std::string_view title);
}
