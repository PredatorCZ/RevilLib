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
#include "revil/re_asset.hpp"
#include "spike/type/pointer.hpp"
#include "spike/uni/common.hpp"
#include "spike/uni/deleter_hybrid.hpp"
#include <span>
#include <vector>

using namespace revil;

template <class C> struct REArray {
  esPointerX64<C> ptr;
  int32 numItems;
};

struct REAssetBase {
  uint32 assetID, assetFourCC;

  template <class C> static C &Get(char *data) {
    return *reinterpret_cast<C *>(data);
  }
};

class revil::REAssetImpl {
public:
  using Ptr = std::unique_ptr<REAssetImpl>;
  std::string internalBuffer;
  char *buffer = nullptr;
  void Load(BinReaderRef rd);
  static Ptr Create(REAssetBase base);
  void Assign(REAssetBase *data);
  virtual void Fixup(std::vector<void *> &ptrStore) = 0;
  virtual void Build() = 0;
  virtual uni::BaseElementConst AsMotion() const { return {}; }
  virtual uni::BaseElementConst AsMotions() const { return {}; }
  virtual uni::BaseElementConst AsSkeletons() const { return {}; }
  virtual ~REAssetImpl() = default;
};

class ProcessFlags {
public:
  char *base;
  std::vector<void *> *ptrStore;
};

template <class C> void RE_EXTERN ProcessClass(C &input, ProcessFlags flags);
