/*  ARCConvert
    Copyright(C) 2021-2022 Lukas Cone

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
#include "datas/binreader.hpp"
#include "datas/binwritter.hpp"
#include "datas/master_printer.hpp"
#include "datas/stat.hpp"
#include "project.h"
#include <atomic>
#include <mutex>
#include <thread>

static struct ARCMake : ReflectorBase<ARCMake> {
  std::string title;
  Platform platform = Platform::Auto;
  bool forceZLIBHeader = false;
} settings;

REFLECT(CLASS(ARCMake),
        MEMBER(title, "t", ReflDesc{"Set title for correct archive handling."}),
        MEMBER(platform, "p",
               ReflDesc{"Set platform for correct archive handling."}),
        MEMBERNAME(forceZLIBHeader, "force-zlib-header", "z",
                   ReflDesc{"Force ZLIB header for files that won't be "
                            "compressed. (Some platforms only)"}));

static AppInfo_s appInfo{
    .header = ARCConvert_DESC " v" ARCConvert_VERSION ", " ARCConvert_COPYRIGHT
                              "Lukas Cone",
    .settings = reinterpret_cast<ReflectorFriend *>(&settings),
};

AppInfo_s *AppInitModule() {
  return &appInfo;
}

struct AFile {
  std::string path;
  size_t offset;
  uint32 hash;
  uint32 uSize;
  uint32 cSize;
};

struct Stream {
  std::string streamPath;
  BinWritter_t<BinCoreOpenMode::NoBuffer> streamStore;
  std::vector<AFile> files;

  Stream(std::string &&path)
      : streamPath(std::move(path)), streamStore(streamPath) {}
};

struct ArcMakeContext : AppPackContext {
  std::string outArc;
  std::map<std::thread::id, Stream> streams;
  const TitleSupport *ts;
  static inline std::atomic_uint32_t numFiles; // fugly

  Stream &NewStream() {
    static std::mutex streamsMutex;
    auto thisId = std::this_thread::get_id();
    uint32 threadId = reinterpret_cast<uint32 &>(thisId);
    std::string path = outArc + std::to_string(threadId) + ".data";

    {
      std::lock_guard<std::mutex> lg(streamsMutex);
      streams.emplace(thisId, std::move(path));
      return streams.at(thisId);
    }
  }

  ArcMakeContext() = default;
  ArcMakeContext(const std::string &path, const AppPackStats &)
      : outArc(path),
        ts(revil::GetTitleSupport(settings.title, settings.platform)) {}
  ArcMakeContext &operator=(ArcMakeContext &&) = default;

  void SendFile(std::string_view path, std::istream &stream) override {
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

    auto found = streams.find(std::this_thread::get_id());
    Stream *tStream = nullptr;

    if (es::IsEnd(streams, found)) {
      tStream = &NewStream();
    } else {
      tStream = &found->second;
    }

    auto &streamStore = tStream->streamStore;
    AFile curFile;
    curFile.offset = streamStore.Tell();
    curFile.hash = hash;
    curFile.uSize = streamSize;
    curFile.path = noExt;

    bool processed = false;
    size_t compressedSize = streamSize;
    const size_t minFileSize =
        appInfo.internalSettings->compressSettings.minFileSize;
    const size_t ratioThreshold =
        appInfo.internalSettings->compressSettings.ratioThreshold;
    const size_t verbosityLevel = appInfo.internalSettings->verbosity;

    if (streamSize > minFileSize || settings.forceZLIBHeader) {
      buffer.resize(streamSize);
      stream.read(&buffer[0], streamSize);
    } else {
      outBuffer.resize(streamSize);
      stream.read(&outBuffer[0], streamSize);
      processed = true;
    }

    if (!processed && streamSize > minFileSize) {
      compressedSize = CompressData(buffer, Z_BEST_COMPRESSION);

      uint32 ratio = ((float)compressedSize / (float)streamSize) * 100;

      if (ratio <= ratioThreshold) {
        processed = true;
      }
    }

    [&] {
      if (!processed) { // compressed with failed ratio or uncompressed data
        if (compressedSize != streamSize && verbosityLevel) {
          printline("Ratio fail "
                    << ((float)compressedSize / (float)streamSize) * 100
                    << "%% for " << path);
        }

        if (settings.forceZLIBHeader) {
          compressedSize = CompressData(buffer, Z_NO_COMPRESSION);
        } else { // compressed with failed ratio
          compressedSize = streamSize;
          streamStore.WriteBuffer(buffer.data(), compressedSize);
          return;
        }
      }

      streamStore.WriteBuffer(outBuffer.data(), compressedSize);
    }();

    curFile.cSize = compressedSize;
    tStream->files.emplace_back(std::move(curFile));
  }

  void Finish() override {
    BinWritter wr(outArc);
    ARCBase arc;
    arc.numFiles = numFiles;
    arc.version = ts->arc.version;

    if (arc.version < 10 || ts->arc.xmemOnly) {
      wr.Write(arc);
    } else {
      ARC arcEx{arc};
      wr.Write(arcEx);
    }

    const size_t dataOffset =
        wr.Tell() + arc.numFiles * ts->arc.extendedFilePath
            ? sizeof(ARCExtendedFile)
            : sizeof(ARCFile);

    std::vector<AFile> files;
    size_t curOffset = dataOffset;

    for (auto &[_, stream] : streams) {
      auto &tFiles = stream.files;
      std::transform(tFiles.begin(), tFiles.end(), std::back_inserter(files),
                     [&](auto &&item) {
                       item.offset += curOffset;
                       return std::move(item);
                     });
      curOffset = dataOffset + tFiles.back().cSize + tFiles.back().offset;
    }

    auto WriteFile = [&](auto cFile, auto &f) {
      cFile.offset = f.offset;
      cFile.typeHash = f.hash;
      memcpy(cFile.fileName, f.path.data(), f.path.size());
      cFile.uncompressedSize = f.uSize;
      cFile.compressedSize = f.cSize;

      if (settings.platform == Platform::Win32) {
        std::replace(std::begin(cFile.fileName), std::end(cFile.fileName), '/',
                     '\\');
      }
    };

    if (ts->arc.extendedFilePath) {
      for (auto &f : files) {
        ARCExtendedFile cFile{};
        WriteFile(cFile, f);
      }
    } else {
      for (auto &f : files) {
        ARCFile cFile{};
        WriteFile(cFile, f);
      }
    }

    for (auto &[_, stream] : streams) {
      es::Dispose(stream.streamStore);
      BinReader_t<BinCoreOpenMode::NoBuffer> rd(stream.streamPath);
      const size_t fSize = rd.GetSize();
      char buffer[0x80000];
      const size_t numBlocks = fSize / sizeof(buffer);
      const size_t restBytes = fSize % sizeof(buffer);

      for (size_t b = 0; b < numBlocks; b++) {
        rd.Read(buffer);
        wr.Write(buffer);
      }

      if (restBytes) {
        rd.ReadBuffer(buffer, restBytes);
        wr.WriteBuffer(buffer, restBytes);
      }

      es::RemoveFile(stream.streamPath);
    }
  }
};

AppPackContext *AppNewArchive(const std::string &folder,
                              const AppPackStats &stats) {
  auto file = folder;
  while (file.back() == '/') {
    file.pop_back();
  }

  file += ".arc";
  return new ArcMakeContext(file, stats);
}
