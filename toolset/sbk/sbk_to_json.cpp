/*  SBK2JSON
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

#include "datas/app_context.hpp"
#include "datas/binreader_stream.hpp"
#include "datas/binwritter_stream.hpp"
#include "datas/except.hpp"
#include "datas/master_printer.hpp"
#include "datas/reflector.hpp"
#include "project.h"
#include <vector>

std::string_view filters[]{
    ".sbk$",
    ".sbkr$",
};

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = SBK2JSON_DESC " v" SBK2JSON_VERSION ", " SBK2JSON_COPYRIGHT
                            "Lukas Cone",
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

struct UnkStruct0 {
  uint32 unk[6];
};

struct UnkStruct1 {
  std::string soundPath;
  uint32 unk[20];

  void Read(BinReaderRef rd) {
    rd.ReadString(soundPath);
    rd.Read(unk);
  }
};

struct Header {
  static constexpr uint32 ID = CompileFourCC("SBKR");
  uint32 id;
  uint32 version;
  uint32 unkCount0;
  uint32 unkCount1;
  uint32 unkCount2;
};

void AppProcessFile(AppContext *ctx) {
  BinReaderRef rd(ctx->GetStream());
  Header hdr;
  rd.Read(hdr);

  if (hdr.id != Header::ID) {
    throw es::InvalidHeaderError(hdr.id);
  }

  if (hdr.version != 4) {
    throw es::InvalidVersionError(hdr.version);
  }

  std::vector<UnkStruct0> str0;

  rd.ReadContainer(str0, hdr.unkCount0);

  std::vector<UnkStruct1> str1;

  rd.ReadContainer(str1, hdr.unkCount1);

  for (auto &s : str1) {
    printinfo(s.soundPath);
  }
}
