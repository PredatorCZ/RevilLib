/*  FPKExtract
    Copyright(C) 2024 Lukas Cone

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

std::string_view filters[]{
    ".fpk$",
};

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = FPKExtract_DESC " v" FPKExtract_VERSION ", " FPKExtract_COPYRIGHT
                              "Lukas Cone",
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

static const uint32 ID = CompileFourCC("FPK");

struct FPKHeader {
  uint32 id;
  uint32 null[2];
  uint16 version;
  uint16 numFiles;
};

struct FPKFile {
  char path[0x40];
  uint32 unk;
  uint32 size0;
  uint32 size1;
  uint32 offset;
};

void AppProcessFile(AppContext *ctx) {
  BinReaderRef rd(ctx->GetStream());
  FPKHeader hdr;
  rd.Read(hdr);

  if (hdr.id != ID) {
    throw es::InvalidHeaderError(hdr.id);
  }

  if (hdr.version != 2) {
    throw es::InvalidVersionError(hdr.version);
  }

  std::vector<FPKFile> files;
  rd.ReadContainer(files, hdr.numFiles);

  auto ectx = ctx->ExtractContext();

  if (ectx->RequiresFolders()) {
    for (auto &f : files) {
      const char *fileName = f.path;
      while (*fileName == '/') {
        fileName++;
      }

      AFileInfo finf(fileName);

      ectx->AddFolderPath(std::string(finf.GetFolder()));
    }

    ectx->GenerateFolders();
  }

  std::string buffer;

  for (auto &f : files) {
    rd.Seek(f.offset);
    rd.ReadContainer(buffer, f.size0);
    const char *fileName = f.path;
    while (*fileName == '/') {
      fileName++;
    }
    ectx->NewFile(fileName);
    ectx->SendData(buffer);
  }
}
