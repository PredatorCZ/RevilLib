/*  Revil Format Library
    Copyright(C) 2017-2020 Lukas Cone

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

#include "datas/macroLoop.hpp"
#include "motion_list_99.hpp"
#include <unordered_map>

#define EVAL_MASTER(classname)                                                 \
  {classname::ID | (classname::VERSION << 32), creator<classname>},

template <class C> REAsset_internal *creator() { return new C(); }

static const std::unordered_map<uint64, REAsset_internal *(*)()> assetRegistry =
    {StaticFor(EVAL_MASTER, REMotlist60Asset, REMotlist85Asset,
               REMotlist99Asset, REMotion43Asset, REMotion78Asset,
               REMotion65Asset)};

REAsset_internal *REAsset_internal::Create(REAssetBase &base) {
  uint64 key = base.assetFourCC;
  key |= static_cast<uint64>(base.assetID) << 32;

  if (assetRegistry.count(key)) {
    return assetRegistry.at(key)();
  }

  return nullptr;
}

thread_local std::vector<void *> es::usedPts;