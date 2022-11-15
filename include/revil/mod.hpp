/*  Revil Format Library
    Copyright(C) 2017-2021 Lukas Cone

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
#include "datas/bincore_fwd.hpp"
#include "revil/platform.hpp"
#include <memory>
#include <string>

namespace revil {

class MODImpl;

enum MODVersion : uint16 {
  X70 = 0x70,
  X99 = 0x99,
  XD3 = 0xD3,
};

struct alignas(8) MODMaker {
  MODVersion version;
  bool swappedEndian = false;
  bool x64 = false;
  Platform platform = Platform::Auto;

  bool operator<(const MODMaker &i0) const;
};

struct alignas(8) BoneIndex {
  uint32 id;
  uint32 motIndex;

  BoneIndex() = default;
  BoneIndex(size_t input) : id(input), motIndex(input >> 32) {}
  BoneIndex(uint32 id_, uint32 motIndex_) : id(id_), motIndex(motIndex_) {}
  operator size_t() const { return reinterpret_cast<const size_t &>(*this); }
};

struct alignas(8) LODIndex {
  bool lod1, lod2, lod3;

  LODIndex() = default;
  LODIndex(size_t input)
      : lod1(input & 0xff), lod2((input >> 8) & 0xff),
        lod3((input >> 16) & 0xff) {}
  operator size_t() const { return reinterpret_cast<const size_t &>(*this); }
};

class ES_EXPORT MOD {
public:
  MOD();
  MOD(MODMaker make);
  MOD(MOD &&);
  ~MOD();
  /* Castable into:
  uni::Element<const uni::Skeleton>
  uni::Element<const unk::Model>
  */
  template <class C> C As() const;
  void Load(const std::string &fileName);
  void Load(BinReaderRef_e rd);
  void Save(BinWritterRef wr);

private:
  std::unique_ptr<MODImpl> pi;
};

} // namespace revil
