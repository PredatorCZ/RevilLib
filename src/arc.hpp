/*  Revil Format Library
    Copyright(C) 2020-2023 Lukas Cone

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
#include "spike/except.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/type/bitfield.hpp"
#include <tuple>
#include <vector>

struct ARCFileSize {
  using Size = BitMemberDecl<0, 29>;
  using Flags = BitMemberDecl<1, 3>;
  using Type = BitFieldType<uint32, Size, Flags>;

  Type sizeAndFlags;

  ARCFileSize() { sizeAndFlags.Set<Flags>(2); }

  operator uint32() const { return sizeAndFlags.Get<Size>(); }
  ARCFileSize &operator=(uint32 sz) {
    sizeAndFlags.Set<Size>(sz);
    return *this;
  }

  void SwapEndian(bool outWay) { FByteswapper(sizeAndFlags, outWay); };
};

struct ARCFile {
  char fileName[0x40];
  uint32 typeHash;
  uint32 compressedSize;
  ARCFileSize uncompressedSize;
  uint32 offset;

  void SwapEndian() {
    FByteswapper(typeHash);
    FByteswapper(compressedSize);
    FByteswapper(uncompressedSize);
    FByteswapper(offset);
  }
};

struct ARCFileV4 : ARCFile {
  void SwapEndian() {
    FByteswapper(typeHash);
    FByteswapper(compressedSize);
    FByteswapper(reinterpret_cast<uint32 &>(uncompressedSize));
    FByteswapper(offset);
  }
};

struct ARCExtendedFile {
  char fileName[0x80];
  uint32 typeHash;
  uint32 compressedSize;
  ARCFileSize uncompressedSize;
  uint32 offset;

  void SwapEndian() {
    FByteswapper(typeHash);
    FByteswapper(compressedSize);
    FByteswapper(uncompressedSize);
    FByteswapper(offset);
  }
};

static constexpr uint32 ARCID = CompileFourCC("ARC");
static constexpr uint32 CRAID = CompileFourCC("\0CRA");

struct ARCBase {
  uint32 id = ARCID;
  uint16 version = 7;
  uint16 numFiles;

  void SwapEndian() {
    FByteswapper(id);
    FByteswapper(version);
    FByteswapper(numFiles);
  }
};

struct ARC : ARCBase {
  int32 LZXTag = 0;

  bool IsLZX() const { return version == 0x11 && LZXTag > 0; }
};

using ARCFiles = std::vector<ARCFile>;
using ARCExtendedFiles = std::vector<ARCExtendedFile>;

auto ReadARC(BinReaderRef_e rd) {
  ARC hdr;
  rd.Read(hdr);

  if (hdr.id == CRAID) {
    rd.SwapEndian(true);
    hdr.SwapEndian();
  } else if (hdr.id != ARCID) {
    throw es::InvalidHeaderError(hdr.id);
  }

  if (!hdr.IsLZX()) {
    rd.Skip(-4);
  }

  ARCFiles files;

  if (hdr.version == 4) {
    rd.ReadContainer(reinterpret_cast<std::vector<ARCFileV4> &>(files),
                     hdr.numFiles);
  } else {
    rd.ReadContainer(files, hdr.numFiles);
  }

  return std::make_tuple(hdr, files);
}

auto ReadExtendedARC(BinReaderRef_e rd) {
  ARC hdr;
  rd.Read(hdr);

  if (hdr.id != ARCID) {
    throw es::InvalidHeaderError(hdr.id);
  }

  rd.Skip(-4);

  ARCExtendedFiles files;
  rd.ReadContainer(files, hdr.numFiles);

  return std::make_tuple(hdr, files);
}
