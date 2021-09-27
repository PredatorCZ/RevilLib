/*  ARCConvert
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

#include "arc_conv.hpp"
#include "datas/binwritter.hpp"
#include "datas/master_printer.hpp"
#include "project.h"

static struct ARCMake : ReflectorBase<ARCMake> {
  std::string title;
  Platform platform = Platform::Auto;
  uint32 compressTreshold = 90;
  uint32 minFileSize = 0x80;
  bool forceZLIBHeader = false;
  bool verbose = false;
} settings;

REFLECT(
    CLASS(ARCMake),
    MEMBER(title, "t", ReflDesc{"Set title for correct archive handling."}),
    MEMBER(platform, "p",
           ReflDesc{"Set platform for correct archive handling."}),
    MEMBERNAME(compressTreshold, "compress-treshold", "c",
               ReflDesc{
                   "Writes compressed data only when compression ratio is less "
                   "than specified threshold [0 - 100]%"}),
    MEMBERNAME(
        minFileSize, "min-file-size", "m",
        ReflDesc{
            "Files that are smaller than specified size won't be compressed."}),
    MEMBERNAME(forceZLIBHeader, "force-zlib-header", "z",
               ReflDesc{"Force ZLIB header for files that won't be "
                        "compressed. (Some platforms only)"}),
    MEMBER(verbose, "v", "Prints more information."));

es::string_view filters[]{
    {},
};

ES_EXPORT AppInfo_s appInfo{
    AppInfo_s::CONTEXT_VERSION,
    AppMode_e::PACK,
    ArchiveLoadType::ALL,
    ARCConvert_DESC " v" ARCConvert_VERSION ", " ARCConvert_COPYRIGHT
                    "Lukas Cone",
    reinterpret_cast<ReflectorFriend *>(&settings),
    filters,
};

struct ArcMakeContext : AppPackContext {
  BinWritter<> wr;
  size_t numFiles = 0;
  const TitleSupport *ts;

  ArcMakeContext() = default;
  ArcMakeContext(const std::string &path, const AppPackStats &stats)
      : wr(path),
        ts(revil::GetTitleSupport(settings.title, settings.platform)) {
    const size_t tocSize =
        ts->arc.extendedFilePath ? sizeof(ARCExtendedFile) : sizeof(ARCFile);
    ARC hdr;
    wr.Write(hdr);

    if (ts->arc.version < 10 || ts->arc.xmemOnly) {
      wr.Skip(-4);
    }

    wr.Push();
    wr.Skip(tocSize * stats.numFiles);
  }
  ArcMakeContext &operator=(ArcMakeContext &&) = default;
  void SendFile(es::string_view path, std::istream &stream) override {
    const size_t extPos = path.find_last_of('.');

    if (extPos == path.npos) {
      printwarning("Skipped (no extension) " << path);
      return;
    }

    auto extension = path.substr(extPos + 1);
    auto hash = GetHash(extension, settings.title, settings.platform);

    if (!hash) {
      printwarning("Skipped (invalid format): " << path);
      return;
    }

    auto noExt = path.substr(0, extPos);

    if (noExt.size() > (ts->arc.extendedFilePath
                            ? sizeof(ARCExtendedFile::fileName)
                            : sizeof(ARCFile::fileName))) {
      printwarning("Skipped (filename too large): " << path);
      return;
    }

    numFiles++;

    if (numFiles > std::numeric_limits<decltype(ARC::numFiles)>::max()) {
      throw std::runtime_error("Filecount exceeded archive limit.");
    }

    stream.seekg(0, std::ios::end);
    const size_t streamSize = stream.tellg();
    stream.seekg(0);

    std::string buffer;
    std::string outBuffer;

    auto CompressData = [&](auto &&buffer, int cType) {
      outBuffer.resize(std::max(buffer.size() + 0x10, size_t(0x8000)));
      z_stream infstream;
      infstream.zalloc = Z_NULL;
      infstream.zfree = Z_NULL;
      infstream.opaque = Z_NULL;
      infstream.avail_in = buffer.size();
      infstream.next_in = reinterpret_cast<Bytef *>(&buffer[0]);
      infstream.avail_out = outBuffer.size();
      infstream.next_out = reinterpret_cast<Bytef *>(&outBuffer[0]);

      deflateInit2(&infstream, cType, Z_DEFLATED, ts->arc.windowSize, 8,
                   Z_DEFAULT_STRATEGY);
      int state = deflate(&infstream, Z_FINISH);
      deflateEnd(&infstream);

      if (state != Z_STREAM_END) {
        throw std::runtime_error("Compression Error!");
      }

      return infstream.total_out;
    };

    bool processed = false;
    size_t compressedSize = streamSize;
    const size_t beginOffset = wr.Tell();

    if (streamSize > settings.minFileSize || settings.forceZLIBHeader) {
      buffer.resize(streamSize);
      stream.read(&buffer[0], streamSize);
    } else {
      outBuffer.resize(streamSize);
      stream.read(&outBuffer[0], streamSize);
      processed = true;
    }

    if (!processed && streamSize > settings.minFileSize) {
      compressedSize = CompressData(buffer, Z_BEST_COMPRESSION);

      uint32 ratio = ((float)compressedSize / (float)streamSize) * 100;

      if (ratio <= settings.compressTreshold) {
        processed = true;
      }
    }

    [&] {
      if (!processed) { // compressed with failed ratio or uncompressed data
        if (compressedSize != streamSize && settings.verbose) {
          printline("Ratio fail "
                    << ((float)compressedSize / (float)streamSize) * 100
                    << "%% for " << path);
        }

        if (settings.forceZLIBHeader) {
          compressedSize = CompressData(buffer, Z_NO_COMPRESSION);
        } else { // compressed with failed ratio
          compressedSize = streamSize;
          wr.WriteBuffer(buffer.data(), compressedSize);
          return;
        }
      }

      wr.WriteBuffer(outBuffer.data(), compressedSize);
    }();

    wr.Push(wr.StackIndex1);
    wr.Pop();

    auto WriteFile = [&](auto &cFile) {
      cFile.offset = beginOffset;
      cFile.typeHash = hash;
      memcpy(cFile.fileName, noExt.data(), noExt.size());
      cFile.uncompressedSize = streamSize;
      cFile.compressedSize = compressedSize;

      if (settings.platform == Platform::WinPC) {
        std::replace(std::begin(cFile.fileName), std::end(cFile.fileName), '/',
                     '\\');
      }

      wr.Write(cFile);
    };

    if (ts->arc.extendedFilePath) {
      ARCExtendedFile cFile{};
      WriteFile(cFile);
    } else {
      ARCFile cFile{};
      if (ts->arc.version == 4) {
        cFile.uncompressedSize.sizeAndFlags.Set<ARCFileSize::Flags>(0);
      }

      WriteFile(cFile);
    }

    wr.Push();
    wr.Pop(wr.StackIndex1);
  }

  void Finish() override {
    wr.Seek(0);
    auto Write = [&](auto &what) {
      what.version = ts->arc.version;
      what.numFiles = numFiles;
      wr.Write(what);
    };

    if (ts->arc.version < 10 || ts->arc.xmemOnly) {
      ARCBase hdr;
      Write(hdr);
    } else {
      ARC hdr;
      Write(hdr);
    }

    es::Dispose(wr);
  }
};

static thread_local ArcMakeContext archive;

AppPackContext *AppNewArchive(const std::string &folder,
                              const AppPackStats &stats) {
  auto file = folder;
  while (file.back() == '/') {
    file.pop_back();
  }

  file += ".arc";
  return &(archive = ArcMakeContext(file, stats));
}
