/*  DLCExtract
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

#include "hfs.hpp"
#include "project.h"
#include "spike/app_context.hpp"
#include "spike/except.hpp"
#include "spike/io/binreader_stream.hpp"
#include <sstream>

std::string_view filters[]{
    ".dat$",
};

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = DLCExtract_DESC " v" DLCExtract_VERSION ", " DLCExtract_COPYRIGHT
                              "Lukas Cone",
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

void AppProcessFile(AppContext *ctx) {
  std::stringstream backup;
  BinReaderRef_e rd(ctx->GetStream());
  uint32 id;
  rd.Push();
  rd.Read(id);

  if (id == SFHID) {
    rd.Pop();
    backup = ProcessHFS(rd);
    rd = BinReaderRef_e(backup);
    rd.Push();
    rd.Read(id);
  }

  if (id != CompileFourCC("DLC")) {
    rd.SwapEndian(true);
    rd.Pop();
    rd.Read(id);
    if (id != CompileFourCC("DLC")) {
      throw es::InvalidHeaderError(id);
    }
  }

  uint8 version;
  rd.Read(version);

  if (version != 3) {
    throw es::InvalidVersionError(version);
  }

  std::string name;
  rd.ReadString(name);

  struct File {
    std::string path;
    uint32 offset;
  };

  std::vector<File> files;
  rd.ReadContainerLambda(files, [](BinReaderRef_e rd, File &file) {
    rd.ReadContainer<uint8>(file.path);
    rd.Read(file.offset);
  });

  auto ectx = ctx->ExtractContext();

  if (ectx->RequiresFolders()) {
    for (auto &f : files) {
      AFileInfo finf(f.path);
      ectx->AddFolderPath(std::string(finf.GetFolder()));
    }

    ectx->GenerateFolders();
  }

  std::string buffer;
  const size_t arSize = rd.GetSize();

  for (size_t curFile = 1; auto &f : files) {
    rd.Seek(f.offset);
    const size_t fileEnd =
        curFile >= files.size() ? arSize : files.at(curFile).offset;
    const size_t fileSize = fileEnd - f.offset;
    rd.ReadContainer(buffer, fileSize);
    ectx->NewFile(f.path);
    ectx->SendData(buffer);
    curFile++;
  }
}
