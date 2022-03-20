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
#include "datas/binreader_stream.hpp"
#include <sstream>

static constexpr uint32 SFHID = CompileFourCC("\0SFH");

struct HFS {
  uint32 id;
  uint16 unk0;
  uint16 unk1;
  uint32 fileSize;
  uint32 null;

  void SwapEndian() {
    FByteswapper(id);
    FByteswapper(unk0);
    FByteswapper(unk1);
    FByteswapper(fileSize);
  }
};

std::stringstream ProcessHFS(BinReaderRef_e rd) {
  std::stringstream str;
  constexpr size_t chunkSize = 0x20000;
  char tempBuffer[chunkSize];
  HFS hdr;
  rd.Read(hdr);

  if (hdr.id == SFHID) {
    hdr.SwapEndian();
  }

  int64 fileSize = hdr.fileSize;

  while (true) {
    if (fileSize < chunkSize) {
      rd.ReadBuffer(tempBuffer, fileSize);
      str.write(tempBuffer, fileSize);
      rd.ApplyPadding(16);
      rd.Skip(16);
      break;
    }

    rd.ReadBuffer(tempBuffer, chunkSize);
    str.write(tempBuffer, chunkSize - 16);
    fileSize -= chunkSize - 16;
  }

  return str;
}
