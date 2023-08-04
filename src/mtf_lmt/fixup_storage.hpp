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
#include "spike/io/binwritter_stream.hpp"

struct LMTFixupStorage {
  struct _data {
    uint32 from, to;
  };

  std::vector<_data> fixupStorage;
  uint32 toIter = 0;

  void SaveFrom(size_t offset) {
    fixupStorage.push_back({static_cast<uint32>(offset), 0});
  }

  void SaveTo(BinWritterRef wr) {
    fixupStorage[toIter++].to = static_cast<uint32>(wr.Tell());
  }

  void FixupPointers(BinWritterRef wr, bool as64bit) {
    size_t savepos = wr.Tell();

    if (as64bit)
      for (auto &f : fixupStorage) {
        wr.Seek(f.from);
        wr.Write<uint64>(f.to);
      }
    else
      for (auto &f : fixupStorage) {
        wr.Seek(f.from);
        wr.Write(f.to);
      }

    wr.Seek(savepos);
  }

  void SkipTo() { toIter++; }
};
