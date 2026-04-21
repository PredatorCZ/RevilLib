/*  Revil Format Library
    Copyright(C) 2020-2026 Lukas Cone

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

#include "revil/arc.hpp"
#include "arc.hpp"
#include "hfs.hpp"
#include "revil/hashreg.hpp"
#include "spike/crypto/blowfish.h"
#include "spike/io/fileinfo.hpp"
#include "spike/master_printer.hpp"
#include <algorithm>
#include <set>

#include "lzx.h"
#include "mspack.h"
#include "zlib.h"

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
                          char *outBuffer, uint32 uncompressedSize,
                          uint32 wBits) {
  mspack_file inStream{};
  mspack_file outStream{};
  inStream.buffer = reinterpret_cast<uint8 *>(inBuffer);
  inStream.bufferSize = compressedSize;
  outStream.buffer = reinterpret_cast<uint8 *>(outBuffer);
  outStream.bufferSize = uncompressedSize;

  lzxd_stream *lzxd = lzxd_init(&mspackSystem, &inStream, &outStream, wBits, 0,
                                1 << wBits, uncompressedSize, false);

  int retVal = lzxd_decompress(lzxd, uncompressedSize);

  if (retVal != MSPACK_ERR_OK) {
    throw std::runtime_error("LZX decompression error " +
                             std::to_string(retVal));
  }

  lzxd_free(lzxd);
}

#pragma endregion

auto ReadARCC(BinReaderRef_e rd, BlowfishEncoder &enc) {
  ARC hdr;
  rd.Read(hdr);
  rd.Skip(-4);

  if (hdr.id != ARCCID) {
    throw es::InvalidHeaderError(hdr.id);
  }

  ARCFiles files;
  rd.ReadContainer(files, hdr.numFiles);

  auto buffer = reinterpret_cast<char *>(files.data());
  size_t bufferSize = sizeof(ARCFile) * hdr.numFiles;

  enc.Decode(buffer, bufferSize);

  return std::make_tuple(hdr, files);
}

void revil::EnumerateArchive(BinReaderRef_e rd, Platform platform,
                             std::string_view title,
                             std::function<AppExtractContext *()> demandContext,
                             const std::set<uint32> &classFilter,
                             bool filterIsBlackList) {
  uint32 id;
  rd.Push();
  rd.Read(id);
  rd.Pop();
  ARC hdr;

  std::stringstream backup;
  if (id == SFHID) {
    backup = ProcessHFS(rd);
    rd = BinReaderRef_e(backup);
    rd.Push();
    rd.Read(id);
    rd.Pop();
  }

  Platform autoPlatform = id == CRAID ? Platform::PS3 : Platform::Win32;

  if (platform != Platform::Auto) {
    if (revil::IsPlatformBigEndian(autoPlatform) !=
        revil::IsPlatformBigEndian(platform)) {
      printwarning("Platform setting mistmatch, using fallback platform: "
                   << (id == CRAID ? "PS3" : "Win32"));
    }
  } else {
    platform = autoPlatform;
  }

  BlowfishEncoder enc;

  auto WriteFiles = [&](auto &files) {
    auto ectx = demandContext();
    if (ectx->RequiresFolders()) {
      for (auto &f : files) {
        if (classFilter.size() > 0 &&
            classFilter.contains(f.typeHash) == filterIsBlackList) {
          continue;
        }

        AFileInfo inf(f.fileName);
        std::string cFolder(inf.GetFolder());
        std::transform(cFolder.begin(), cFolder.end(), cFolder.begin(),
                       [](char c) { return tolower(c); });
        ectx->AddFolderPath(cFolder);
      }

      ectx->GenerateFolders();
    }

    std::string inBuffer;
    std::string outBuffer;
    [&inBuffer, &outBuffer, &files] {
      size_t maxSize = 0;
      size_t maxSizeUnc = 0;

      for (auto &f : files) {
        if (f.uncompressedSize > maxSizeUnc) {
          maxSizeUnc = f.uncompressedSize;
        }

        if (f.compressedSize > maxSize) {
          maxSize = f.compressedSize;
        }
      }

      if (maxSizeUnc < 0x8000) {
        maxSizeUnc = 0x8000;
      }

      inBuffer.resize(maxSize);
      outBuffer.resize(maxSizeUnc);
    }();

    for (auto &f : files) {
      if (!f.compressedSize) {
        continue;
      }

      if (classFilter.size() > 0 &&
          classFilter.contains(f.typeHash) == filterIsBlackList) {
        continue;
      }

      rd.Seek(f.offset);

      if (platform == Platform::PS3 && f.compressedSize == f.uncompressedSize) {
        rd.ReadBuffer(&outBuffer[0], f.compressedSize);

        if (id == ARCCID) {
          enc.Decode(&outBuffer[0], f.compressedSize);
        }
      } else {
        rd.ReadBuffer(&inBuffer[0], f.compressedSize);

        if (id == ARCCID) {
          enc.Decode(&inBuffer[0], f.compressedSize);
        }

        if (hdr.IsLZX()) {
          DecompressLZX(&inBuffer[0], f.compressedSize, &outBuffer[0],
                        f.uncompressedSize, id == ARCID ? 17 : 15);
        } else {
          z_stream infstream;
          infstream.zalloc = Z_NULL;
          infstream.zfree = Z_NULL;
          infstream.opaque = Z_NULL;
          infstream.avail_in = f.compressedSize;
          infstream.next_in = reinterpret_cast<Bytef *>(&inBuffer[0]);
          infstream.avail_out = outBuffer.size();
          infstream.next_out = reinterpret_cast<Bytef *>(&outBuffer[0]);
          inflateInit(&infstream);
          int state = inflate(&infstream, Z_FINISH);
          inflateEnd(&infstream);

          if (state < 0) {
            throw std::runtime_error(infstream.msg);
          }
        }
      }

      auto ext = revil::GetExtension(f.typeHash, title, platform);
      std::string filePath = f.fileName;
      std::transform(filePath.begin(), filePath.end(), filePath.begin(),
                     [](char c) { return tolower(c); });
      filePath.push_back('.');

      if (ext.empty()) {
        char buffer[0x10]{};
        snprintf(buffer, sizeof(buffer), "%.8" PRIX32, f.typeHash);
        filePath += buffer;
      } else {
        filePath.append(ext);
      }

      ectx->NewFile(filePath);
      ectx->SendData({outBuffer.data(), f.uncompressedSize});
    }
  };

  auto ts = revil::GetTitleSupport(title, platform);

  if (ts->arc.flags & revil::DbArc_ExtendedPath) {
    ARCExtendedFiles files;
    std::tie(hdr, files) = ReadExtendedARC(rd);
    WriteFiles(files);
  } else {
    ARCFiles files;
    if (id == ARCCID) {
      std::string_view key(ts->arc.key);

      if (key.empty()) {
        throw es::RuntimeError(
            "Encrypted archives not supported for this title");
      }
      enc.SetKey(key);
      std::tie(hdr, files) = ReadARCC(rd, enc);
    } else {
      std::tie(hdr, files) = ReadARC(rd);
    }
    WriteFiles(files);
  }
}

size_t revil::CompressZlib(std::string_view inBuffer, std::string &outBuffer,
                           int windowSize, int level) {
  z_stream infstream;
  infstream.zalloc = Z_NULL;
  infstream.zfree = Z_NULL;
  infstream.opaque = Z_NULL;
  infstream.avail_in = inBuffer.size();
  infstream.next_in =
      const_cast<Bytef *>(reinterpret_cast<const Bytef *>(inBuffer.data()));
  infstream.avail_out = outBuffer.size();
  infstream.next_out = reinterpret_cast<Bytef *>(outBuffer.data());

  deflateInit2(&infstream, level, Z_DEFLATED, windowSize, 8,
               Z_DEFAULT_STRATEGY);
  int state = deflate(&infstream, Z_FINISH);
  deflateEnd(&infstream);

  if (state != Z_STREAM_END) {
    throw es::RuntimeError("Compression Error!");
  }

  return infstream.total_out;
}
