/*  UDASExtract
    Copyright(C) 2021-2022 Lukas Cone
    Based on mariokart64n's research.

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

#include "project.h"
#include "spike/app_context.hpp"
#include "spike/except.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/reflect/reflector.hpp"
#include "spike/type/pointer.hpp"
#include <sstream>

#include "lzx.h"
#include "mspack.h"

#pragma region XMemDecompress

struct mspack_file {
  uint8 *buffer;
  uint32 bufferSize;
  uint32 position;
  uint32 rest;
};

// https://github.com/gildor2/UEViewer/blob/master/Unreal/UnCoreCompression.cpp#L90
static int mspack_read(mspack_file *file, void *buffer, int bytes) {
  if (!file->rest) {
    // read block header
    if (file->buffer[file->position] == 0xFF) {
      // [0]   = FF
      // [1,2] = uncompressed block size
      // [3,4] = compressed block size
      file->rest = (file->buffer[file->position + 3] << 8) |
                   file->buffer[file->position + 4];
      file->position += 5;
    } else {
      // [0,1] = compressed size
      file->rest = (file->buffer[file->position + 0] << 8) |
                   file->buffer[file->position + 1];
      file->position += 2;
    }

    if (file->rest > file->bufferSize - file->position) {
      file->rest = file->bufferSize - file->position;
    }
  }

  if (bytes > file->rest) {
    bytes = file->rest;
  }

  if (bytes <= 0) {
    return 0;
  }

  memcpy(buffer, file->buffer + file->position, bytes);
  file->position += bytes;
  file->rest -= bytes;

  return bytes;
}

static int mspack_write(mspack_file *file, void *buffer, int bytes) {
  if (bytes <= 0) {
    return 0;
  }

  memcpy(file->buffer + file->position, buffer, bytes);
  file->position += bytes;
  return bytes;
}

static mspack_system mspackSystem{
    nullptr,                                                     // open
    nullptr,                                                     // close
    mspack_read,                                                 // read
    mspack_write,                                                // write
    nullptr,                                                     // seek
    nullptr,                                                     // tell
    nullptr,                                                     // message
    [](mspack_system *, size_t bytes) { return malloc(bytes); }, // alloc
    free,                                                        // free
    [](void *src, void *dst, size_t bytes) { memcpy(dst, src, bytes); }, // copy
};

static void DecompressLZX(char *inBuffer, uint32 compressedSize,
                          char *outBuffer, uint32 uncompressedSize) {
  mspack_file inStream{};
  mspack_file outStream{};
  inStream.buffer = reinterpret_cast<uint8 *>(inBuffer);
  inStream.bufferSize = compressedSize;
  outStream.buffer = reinterpret_cast<uint8 *>(outBuffer);
  outStream.bufferSize = uncompressedSize;

  lzxd_stream *lzxd = lzxd_init(&mspackSystem, &inStream, &outStream, 17, 0,
                                0x40000, uncompressedSize, false);

  int retVal = lzxd_decompress(lzxd, uncompressedSize);

  if (retVal != MSPACK_ERR_OK) {
    throw std::runtime_error("LZX decompression error " +
                             std::to_string(retVal));
  }

  lzxd_free(lzxd);
}

#pragma endregion

std::string_view filters[]{
    //".udas$",
    ".udas.lfs$",
};

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = UDASExtract_DESC " v" UDASExtract_VERSION
                               ", " UDASExtract_COPYRIGHT "Lukas Cone",
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

struct LFSChunk {
  uint16 compressedSize;
  uint16 uncompressedSize;
  uint32 offset;
};

struct LFSHeader {
  static constexpr uint32 ID = CompileFourCC("RDLX");
  uint32 id;
  uint32 id2;
  uint32 uncompressedSize;
  uint32 compressedSize;
};

enum DASEntryType {
  End = -1,
  FileTable,
  Unk0,
  Unk1,
  Unk2,
  Child,
};

struct DASEntry {
  DASEntryType type;
  uint32 size;
  uint32 null;
  es::PointerX86<char> data;
  uint32 unk[4];
};

struct DASHeader {
  static constexpr uint32 ID = 0xCAB6BE20;
  uint32 id[8]{ID, ID, ID, ID, ID, ID, ID, ID};
  DASEntry entries[1];
};

struct DAT {
  uint32 numFiles;
  uint32 null[3];
  es::PointerX86<char> data[1];

  uint32 GetType(uint32 index) {
    return reinterpret_cast<uint32 &>(data[numFiles + index]);
  }
};

void ProcessClass(DAT &item) {
  char *root = reinterpret_cast<char *>(&item);
  for (size_t i = 0; i < item.numFiles; i++) {
    item.data[i].Fixup(root);
  }
}

void ProcessClass(DASHeader &hdr);

void ProcessClass(DASEntry &item, char *root) {
  item.data.Fixup(root);
  if (item.type == DASEntryType::Child) {
    DASHeader *child = reinterpret_cast<DASHeader *>(item.data.Get());
    ProcessClass(*child);
  } else if (item.type == DASEntryType::FileTable) {
    DAT *dat = reinterpret_cast<DAT *>(item.data.Get());
    ProcessClass(*dat);
  }
}

void ProcessClass(DASHeader &hdr) {
  if (hdr.id[0] != hdr.ID) {
    throw es::InvalidHeaderError(hdr.id[0]);
  }

  char *root = reinterpret_cast<char *>(&hdr);

  DASEntry *curEntry = hdr.entries;

  while (curEntry->type != DASEntryType::End) {
    ProcessClass(*curEntry, root);
    curEntry++;
  }
}

void ExtractData(DAT &item, uint32 endPos, AppExtractContext *ectx,
                 std::string curPath) {

  auto FileName = [&](size_t id) {
    uint32 type = item.GetType(id);
    std::string_view typeStr(reinterpret_cast<const char *>(&type));
    if (typeStr.size() > 4) {
      typeStr = typeStr.substr(0, 4);
    }
    ectx->NewFile(curPath + "_" + std::to_string(id) + "." +
                  std::string(typeStr));
  };

  for (size_t i = 0; i < item.numFiles - 1; i++) {
    char *begin = item.data[i];
    char *end = item.data[i + 1];
    std::string_view fData{begin, end};
    if (!fData.empty()) {
      FileName(i);
      ectx->SendData(fData);
    }
  }

  char *begin = item.data[item.numFiles - 1];
  char *end = reinterpret_cast<char *>(&item) + endPos;
  std::string_view fData{begin, end};
  if (!fData.empty()) {
    FileName(item.numFiles - 1);
    ectx->SendData(fData);
  }
}

void ExtractData(DASHeader &hdr, AppExtractContext *ectx, std::string curPath);

void ExtractData(DASEntry &item, AppExtractContext *ectx, std::string curPath) {
  if (item.type == DASEntryType::Child) {
    DASHeader *child = reinterpret_cast<DASHeader *>(item.data.Get());
    ExtractData(*child, ectx, curPath);
  } else if (item.type == DASEntryType::FileTable) {
    DAT *dat = reinterpret_cast<DAT *>(item.data.Get());
    ExtractData(*dat, item.size, ectx, curPath);
  } else if (item.size) {
    ectx->NewFile(curPath);
    ectx->SendData({item.data.Get(), item.size});
  }
}

void ExtractData(DASHeader &hdr, AppExtractContext *ectx, std::string curPath) {
  DASEntry *curEntry = hdr.entries;
  size_t curItem = 0;

  while (curEntry->type != DASEntryType::End) {
    ExtractData(*curEntry, ectx, curPath + "_" + std::to_string(curItem));
    curEntry++;
    curItem++;
  }
}

void AppProcessFile(AppContext *ctx) {
  std::string buffer;
  uint32 id;
  ctx->GetType(id);

  if (id == LFSHeader::ID) {
    BinReaderRef rd(ctx->GetStream());
    LFSHeader hdr;
    std::vector<LFSChunk> chunks;
    rd.Read(hdr);
    rd.ReadContainer(chunks);
    char inBuffer[0x10000];
    char outBuffer[0x10000];
    std::stringstream backup;

    for (auto &c : chunks) {
      rd.Seek(c.offset + sizeof(hdr) + 3);
      rd.ReadBuffer(inBuffer, c.compressedSize);
      uint32 uncompressedSize =
          c.uncompressedSize ? c.uncompressedSize : sizeof(outBuffer);
      DecompressLZX(inBuffer, c.compressedSize, outBuffer, uncompressedSize);
      backup.write(outBuffer, uncompressedSize);
    }

    buffer = std::move(backup).str();
    ctx->workingFile = ctx->workingFile.GetFullPathNoExt();
  } else {
    throw es::InvalidHeaderError(id);
  }

  DASHeader *hdr = reinterpret_cast<DASHeader *>(buffer.data());
  ProcessClass(*hdr);
  auto ectx = ctx->ExtractContext();
  ExtractData(*hdr, ectx, std::string(ctx->workingFile.GetFilename()));
}
