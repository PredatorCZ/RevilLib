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

#pragma once
#include "datas/supercore.hpp"
#include "pointer.hpp"
#include "re_asset.hpp"

#include "datas/allocator_hybrid.hpp"
#include <vector>

template <class C> struct REArray {
  REPointerX64<C> ptr;
  int numItems;
};

struct REAssetBase {
  uint assetID, assetFourCC;

  template <class C, class I> static C &Get(I &data) {
    return reinterpret_cast<C &>(data[0]);
  }
};

class REAsset_internal : public REAsset {
public:
  typedef std::vector<char, std::allocator_hybrid<char>> buffer_type;
  buffer_type buffer;
  int Load(BinReader *rd);
  static REAsset_internal *Create(REAssetBase &base);
  void Assign(REAssetBase *data);
  virtual int Fixup() = 0;
  virtual void Build() = 0;
};