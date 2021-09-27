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

#include "../hfs.hpp"
#include "arc.hpp"
#include "datas/binreader.hpp"
#include "datas/binwritter.hpp"
#include "datas/directory_scanner.hpp"
#include "datas/encrypt/blowfish.h"
#include "datas/multi_thread.hpp"
#include "datas/pugiex.hpp"
#include "datas/reflector_xml.hpp"
#include "datas/settings_manager.hpp"
#include "datas/stat.hpp"
#include "lzx.h"
#include "mspack.h"
#include "project.h"
#include "revil/hashreg.hpp"
#include "revil/platform.hpp"
#include "zlib.h"
#include <cinttypes>

#include "../src/mtf_arc/ext_mhs2.hpp"
#include "../src/mtf_arc/util_scan_vfs.hpp"

using namespace revil;

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

REFLECTOR_CREATE(RPlatform, ENUM, 1, CLASS, //
                 Auto, WinPC, PS3, X360, N3DS, CAFE, NSW);

struct CompressionSettings {
  uint32 Compress_Treshold = 90;
  uint32 Min_File_Size = 0x80;
  bool Force_ZLIB_Header = false;
  bool Verbose = false;
  uint32 version = 8;
  uint32 windowSize = 15;
  void ReflectorTag();
};

REFLECTOR_CREATE(
    CompressionSettings, 1, EXTENDED,
    (D, Compress_Treshold,
     "Writes compressed data only when compression ratio is less than "
     "specified threshold [0 - 100]%%"),
    (D, Min_File_Size,
     "Files that are smaller than specified size won't be compressed."),
    (D, Force_ZLIB_Header,
     "Force ZLIB header for files that won't be compressed. (Some platforms "
     "only)"),
    (D, Verbose, "Prints more information."));

static struct ARCConvert : SettingsManager<ARCConvert> {
  bool Generate_Log = false;
  bool Folder_scan_ARC_extract = true;
  bool Folder_per_ARC = true;
  std::string Title;
  RPlatform Platform = RPlatform::Auto;
  CompressionSettings Compression_Settings;
} settings;

REFLECTOR_CREATE(
    ARCConvert, 1, EXTENDED,
    (D, Title, "Set title for correct archive handling."),
    (D, Platform, "Set platform for correct archive handling."),
    (D, Folder_per_ARC,
     "When extracting, create folder that uses arc name as output dir."),
    (D, Folder_scan_ARC_extract,
     "Folder will be scanned for ARC extraction. Otherwise "
     "folders will be used for "
     "ARC creation."),
    (D, Generate_Log,
     "Will generate text log of console output next to "
     "application location."),
    (, Compression_Settings));

static const char appHeader[] = ARCConvert_DESC
    " v" ARCConvert_VERSION ", " ARCConvert_COPYRIGHT "Lukas Cone"
    "\nSimply drag'n'drop files/folders onto application or "
    "use as " ARCConvert_NAME
    " path1 path2 ...\nTool can detect and scan folders.";

static const char configHelp[] = "For settings, edit .config file.";

static const char DDONKey[] =
    "ABB(DF2I8[{Y-oS_CCMy(@<}qR}WYX11M)w[5V.~CbjwM5q<F1Iab+-";
static BlowfishEncoder enc;
static constexpr uint32 ARCCID = CompileFourCC("ARCC");

auto ReadARCC(BinReaderRef rd) {
  ARC hdr;
  rd.Read(hdr);
  rd.Skip(-4);

  if (hdr.id != ARCCID) {
    throw es::InvalidHeaderError(hdr.id);
  }

  ARCFiles files;
  rd.ReadContainer(files, hdr.numFiles);

  auto buffer = reinterpret_cast<char *>(files.begin().operator->());
  size_t bufferSize = sizeof(ARCFile) * hdr.numFiles;

  enc.Decode(buffer, bufferSize);

  return std::make_tuple(hdr, files);
}

void Extract(BinReaderRef rd, const std::string &outDir,
             bool makeFolder = true) {
  std::stringstream backup;
  rd.Push();
  uint32 id;
  rd.Read(id);
  rd.Pop();
  ARC hdr;

  if (id == SFHID) {
    backup = ProcessHFS(rd);
    rd = BinReaderRef(backup);
    rd.Push();
    rd.Read(id);
    rd.Pop();
  }

  Platform platform = id == CRAID ? Platform::PS3 : Platform::WinPC;

  if (settings.Platform != RPlatform::Auto) {
    platform = Platform(settings.Platform);
  }

  AFileInfo fleInfo(outDir);
  std::string extrpath;

  if (makeFolder) {
    extrpath = fleInfo.GetFullPathNoExt();
    extrpath += '/';
    es::mkdir(extrpath);
  } else {
    extrpath = fleInfo.GetFolder();
  }

  auto WriteFiles = [&](auto &files) {
    mkdirs(extrpath, files, [](auto &f) { return f.fileName; });

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

        if (hdr.version == 0x11 && hdr.LZXTag) {
          DecompressLZX(&inBuffer[0], f.compressedSize, &outBuffer[0],
                        f.uncompressedSize);
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

      AFileInfo cfleWrap(f.fileName);
      auto ext = revil::GetExtension(f.typeHash, settings.Title, platform);
      std::string filePath =
          extrpath + cfleWrap.GetFullPath().to_string() + '.';

      if (ext.empty()) {
        char buffer[0x10]{};
        snprintf(buffer, sizeof(buffer), "%.8" PRIX32, f.typeHash);
        filePath += buffer;
      } else {
        filePath += ext.to_string();
      }

      std::ofstream str(filePath, std::ios::out | std::ios::binary);
      if (!str.fail()) {
        str.write(outBuffer.data(), f.uncompressedSize);
        str.close();
      }
    }
  };

  auto ts = revil::GetTitleSupport(settings.Title, Platform(settings.Platform));

  if (ts->arc.extendedFilePath) {
    ARCExtendedFiles files;
    std::tie(hdr, files) = ReadExtendedARC(rd);
    WriteFiles(files);
  } else {
    ARCFiles files;
    if (id == ARCCID) {
      std::tie(hdr, files) = ReadARCC(rd);
    } else {
      std::tie(hdr, files) = ReadARC(rd);
    }
    WriteFiles(files);
  }
}

void Extract(const std::string &fileName, bool makeFolder = true) {
  printline("Extracting " << fileName);
  BinReader rd(fileName);
  Extract(rd, fileName);
}

void Compress(const std::string &folder, CompressionSettings csets) {
  Platform platform = Platform(settings.Platform);
  GetHash("mod", settings.Title, platform);

  DirectoryScanner scan;
  scan.Scan(folder);
  std::vector<std::string *> validFiles;
  std::vector<ARCFile> afiles;
  validFiles.reserve(scan.Files().size());
  afiles.reserve(scan.Files().size());

  for (auto &f : scan) {
    auto cFileNameFull = es::string_view(f);
    cFileNameFull.remove_prefix(folder.size() + 1);
    AFileInfo fInfo(cFileNameFull);
    auto ext = fInfo.GetExtension();
    ext.remove_prefix(1);
    auto hash = GetHash(ext, "dd", platform);

    if (!hash) {
      printwarning("Skipped: " << f);
      continue;
    }

    validFiles.push_back(&f);

    ARCFile cFile;
    cFile.typeHash = hash;

    if (csets.version == 4) {
      cFile.uncompressedSize.sizeAndFlags.Set<ARCFileSize::Flags>(0);
    }

    auto cFileName = fInfo.GetFullPathNoExt();
    memset(cFile.fileName, 0, sizeof(cFile.fileName));
    memcpy(cFile.fileName, cFileName.data(), cFileName.size());

    if (platform == Platform::WinPC) {
      std::replace(std::begin(cFile.fileName), std::end(cFile.fileName), '/',
                   '\\');
    }

    afiles.emplace_back(std::move(cFile));
  }

  BinWritter wr(folder + ".arc");
  ARC hdr;
  hdr.numFiles = afiles.size();
  hdr.version = csets.version;
  wr.Write(hdr);
  wr.Push();
  wr.Skip(sizeof(ARCFile) * hdr.numFiles);

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

    deflateInit2(&infstream, cType, Z_DEFLATED, csets.windowSize, 8,
                 Z_DEFAULT_STRATEGY);
    int state = deflate(&infstream, Z_FINISH);
    deflateEnd(&infstream);

    if (state != Z_STREAM_END) {
      throw std::runtime_error("Compression Error!");
    }

    return infstream.total_out;
  };

  for (size_t i = 0; i < hdr.numFiles; i++) {
    auto &&cFilePath = *validFiles[i];
    std::ifstream str(cFilePath,
                      std::ios::in | std::ios::binary | std::ios::ate);

    if (str.fail()) {
      throw es::FileInvalidAccessError(cFilePath);
    }

    const size_t fileSize = str.tellg();
    str.seekg(0);

    auto &cAfile = afiles[i];
    bool processed = false;
    cAfile.uncompressedSize = fileSize;
    cAfile.compressedSize = fileSize;

    if (fileSize > csets.Min_File_Size || csets.Force_ZLIB_Header) {
      buffer.resize(fileSize);
      str.read(&buffer[0], fileSize);
    } else {
      outBuffer.resize(fileSize);
      str.read(&outBuffer[0], fileSize);
      processed = true;
    }

    str.close();

    if (!processed && fileSize > csets.Min_File_Size) {
      cAfile.compressedSize = CompressData(buffer, Z_BEST_COMPRESSION);

      uint32 ratio =
          ((float)cAfile.compressedSize / (float)cAfile.uncompressedSize) * 100;

      if (ratio <= csets.Compress_Treshold) {
        processed = true;
      }
    }

    cAfile.offset = wr.Tell();

    if (!processed) { // compressed with failed ratio or uncompressed data
      if (cAfile.compressedSize != cAfile.uncompressedSize && csets.Verbose) {
        auto extension =
            GetExtension(cAfile.typeHash, settings.Title, platform);
        printline("Ratio fail " << ((float)cAfile.compressedSize /
                                    (float)cAfile.uncompressedSize) *
                                       100
                                << "%% for " << cAfile.fileName << "."
                                << (extension.empty() ? "" : extension));
      }

      if (csets.Force_ZLIB_Header) {
        cAfile.compressedSize = CompressData(buffer, Z_NO_COMPRESSION);
      } else { // compressed with failed ratio
        cAfile.compressedSize = fileSize;
        wr.WriteBuffer(buffer.data(), fileSize);
        continue;
      }
    }

    wr.WriteBuffer(outBuffer.data(), cAfile.compressedSize);
  }

  wr.Pop();
  wr.WriteContainer(afiles);
}

static std::stringstream titleStream;

void AddTitle(es::string_view titleName) {
  titleStream << "\t\t" << titleName;
  PlatformFlags flags = GetPlatformSupport(titleName);
  bool added = false;
  auto ref = GetReflectedEnum<RPlatform>();

  for (size_t f = 1; f < ref.size(); f++) {
    if (flags[Platform(f)]) {
      if (!added) {
        titleStream << " (" << ref[f];
        added = true;
        continue;
      }
      titleStream << ", " << ref[f];
    }
  }

  if (added) {
    titleStream << ')';
  }

  titleStream << std::endl;
}

void ArcConvInit(const std::string &configName) {
  printer.AddPrinterFunction(UPrintf);
  LinkLogging(UPrintf, true);
  enc.SetKey(DDONKey);
  RegisterReflectedTypes<RPlatform, CompressionSettings>();
  printline(appHeader);

  try {
    auto doc = XMLFromFile(configName);
    ReflectorXMLUtil::LoadV2(settings, doc, true);
  } catch (const es::FileNotFoundError &e) {
  }
  {
    pugi::xml_document doc = {};
    std::stringstream str;
    settings.GetHelp(str);
    str << "\tValid titles: title ( supported platforms )" << std::endl;
    revil::GetTitles(AddTitle);
    str << titleStream.str();

    auto buff = str.str();
    doc.append_child(pugi::node_comment).set_value(buff.data());

    ReflectorXMLUtil::SaveV2a(settings, doc,
                              {ReflectorXMLUtil::Flags_ClassNode,
                               ReflectorXMLUtil::Flags_StringAsAttribute});
    XMLToFile(configName, doc,
              {XMLFormatFlag::WriteBOM, XMLFormatFlag::IndentAttributes});
  }
}

void Dump();

int _tmain(int argc, TCHAR *argv[]) {
  setlocale(LC_ALL, "");
  AFileInfo configInfo(std::to_string(argv[0]));
  auto configName = configInfo.GetFullPathNoExt().to_string();
  auto configPath = configName + ".config";
  ArcConvInit(configPath);

  if (argc < 2) {
    printerror("Insufficient argument count, expected at least 1.");
    printline(configHelp);
    return 1;
  }

  if (IsHelp(argv[1])) {
    printline(configHelp);
    return 0;
  }

  if (settings.Generate_Log) {
    settings.CreateLog(configName);
  }

  std::vector<std::string> files;
  std::vector<std::string> folders;

  for (int a = 1; a < argc; a++) {
    auto fileName = std::to_string(argv[a]);
    auto type = FileType(fileName);

    switch (type) {
    case FileType_e::Directory: {
      if (!settings.Folder_scan_ARC_extract) {
        folders.emplace_back(std::move(fileName));
        break;
      }

      DirectoryScanner sc;
      sc.AddFilter(".arc");
      printline("Scanning: " << fileName);
      sc.Scan(fileName);
      printline("Files found: " << sc.Files().size());

      std::transform(std::make_move_iterator(sc.begin()),
                     std::make_move_iterator(sc.end()),
                     std::back_inserter(files),
                     [](auto &&item) { return std::move(item); });

      break;
    }
    case FileType_e::File:
      files.emplace_back(std::move(fileName));
      break;
    default:
      printerror("Invalid path: " << fileName);
      break;
    }
  }

  printer.PrintThreadID(true);

  RunThreadedQueue(files.size(), [&](size_t index) {
    try {
      Extract(files[index], settings.Folder_per_ARC);
    } catch (const std::exception &e) {
      printerror(e.what());
    }
  });

  if (folders.empty()) {
    return 0;
  }

  RunThreadedQueue(folders.size(), [&](size_t index) {
    try {
      Compress(files[index], {});
    } catch (const std::exception &e) {
      printerror(e.what());
    }
  });

  return 0;
}
