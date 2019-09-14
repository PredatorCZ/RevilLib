/*      Revil Format Library
        Copyright(C) 2017-2019 Lukas Cone

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
#include "datas/binwritter.hpp"

struct LMTFixupStorage {
  struct _data {
    uint from, to;
  };

  std::vector<_data> fixupStorage;
  uint toIter = 0;

  void SaveFrom(uint offset) { fixupStorage.push_back({offset, 0}); }

  void SaveTo(BinWritter *wr) { fixupStorage[toIter++].to = wr->Tell(); }

  void FixupPointers(BinWritter *wr, bool as64bit) {
    size_t savepos = wr->Tell();

    if (as64bit)
      for (auto &f : fixupStorage) {
        wr->Seek(f.from);
        wr->Write<uint64>(f.to);
      }
    else
      for (auto &f : fixupStorage) {
        wr->Seek(f.from);
        wr->Write(f.to);
      }

    wr->Seek(savepos);
  }

  void SkipTo() { toIter++; }
};