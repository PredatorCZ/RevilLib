/*  OBBExtract
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

#include "project.h"
#include "spike/app_context.hpp"
#include "spike/crypto/crc32.hpp"
#include "spike/except.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/io/stat.hpp"
#include "spike/master_printer.hpp"
#include "spike/reflect/reflector.hpp"
#include <charconv>
#include <sstream>

std::string_view filters[]{
    ".obb$",
};

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = OBBExtract_DESC " v" OBBExtract_VERSION ", " OBBExtract_COPYRIGHT
                              "Lukas Cone",
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

static std::map<uint32, std::string_view> FILES;
static es::MappedFile mappedFile;

bool AppInitContext(const std::string &dataFolder) {
  mappedFile = es::MappedFile(dataFolder + "mhs.files");
  std::string_view totalMap(static_cast<const char *>(mappedFile.data),
                            mappedFile.fileSize);

  while (!totalMap.empty()) {
    size_t found = totalMap.find_first_of("\r\n");

    if (found != totalMap.npos) {
      auto sub = totalMap.substr(0, found);
      uint32 crc = ~crc32b(0, sub.data(), sub.size());

      if (FILES.contains(crc) && FILES.at(crc) != sub) {
        printerror("File colision: " << FILES.at(crc) << " vs: " << sub);
      } else {
        FILES.emplace(crc, sub);
      }

      totalMap.remove_prefix(found + 1);

      if (totalMap.front() == '\n') {
        totalMap.remove_prefix(1);
      }
    }
  }

  return true;
}

struct OBBFile {
  uint32 nameHash;
  uint32 offset;
  uint32 size;
  uint32 crc;
};

struct OBBHeader {
  uint32 id;
  uint32 version;
  uint32 numFiles;
  uint32 tocCrc;
};

void AppProcessFile(AppContext *ctx) {
  BinReaderRef rd(ctx->GetStream());
  OBBHeader id;
  rd.Read(id);

  if (id.id != CompileFourCC(".OBB")) {
    throw es::InvalidHeaderError(id.id);
  }

  if (id.version != 1) {
    throw es::InvalidVersionError(id.version);
  }

  std::vector<OBBFile> files;
  rd.ReadContainer(files, id.numFiles);

  auto ectx = ctx->ExtractContext();

  if (ectx->RequiresFolders()) {
    for (auto &f : files) {
      auto filePath = FILES.at(f.nameHash);
      ectx->AddFolderPath(std::string(filePath));
    }

    ectx->GenerateFolders();
  }

  std::string buffer;

  for (auto &f : files) {
    rd.Seek(f.offset);
    rd.ReadContainer(buffer, f.size);

    if (FILES.contains(f.nameHash)) {
      auto filePath = FILES.at(f.nameHash);
      ectx->NewFile(std::string(filePath));
    } else {
      char hexbuffer[0x10]{};
      std::to_chars(std::begin(hexbuffer), std::end(hexbuffer), f.nameHash,
                    0x10);
      std::string fileBuffer = hexbuffer;

      if (buffer.starts_with("TEX")) {
        fileBuffer.append(".tex");
      } else if (buffer.starts_with("MOD")) {
        fileBuffer.append(".mod");
      } else if (buffer.starts_with("LMT")) {
        fileBuffer.append(".lmt");
      } else if (buffer.starts_with("MRL")) {
        fileBuffer.append(".mrl");
      } else if (buffer.starts_with("XFS")) {
        fileBuffer.append(".xfs");
      } else if (buffer.starts_with("FWSE")) {
        fileBuffer.append(".sew");
      } else if (buffer.starts_with("SBKR")) {
        fileBuffer.append(".sbkr");
      } else if (buffer.starts_with("OggS")) {
        fileBuffer.append(".sngw");
      } else if (buffer.starts_with("SPTL")) {
        fileBuffer.append(".sptl");
      } else if (buffer.starts_with("REVR")) {
        fileBuffer.append(".revr_and");
      } else if (buffer.starts_with("SRQR")) {
        fileBuffer.append(".srqr");
      } else if (buffer.starts_with("ARC")) {
        fileBuffer.append(".arc");
      } else if (buffer.starts_with("lyt")) {
        fileBuffer.append(".lyt");
      } else if (buffer.starts_with("lan")) {
        fileBuffer.append(".lan");
      } else if (buffer.starts_with("lmd")) {
        fileBuffer.append(".lmd");
      }

      ectx->NewFile(fileBuffer);
    }

    ectx->SendData(buffer);
  }
}

size_t AppExtractStat(request_chunk requester) {
  auto data = requester(0, 16);
  OBBHeader *hdr = reinterpret_cast<OBBHeader *>(data.data());

  if (hdr->id != CompileFourCC(".OBB")) {
    throw es::InvalidHeaderError(hdr->id);
  }

  if (hdr->version != 1) {
    throw es::InvalidVersionError(hdr->version);
  }

  return hdr->numFiles;
}
