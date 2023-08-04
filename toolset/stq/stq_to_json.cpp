/*  STQ2JSON
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
#include "spike/except.hpp"
#include "spike/master_printer.hpp"
#include "spike/reflect/reflector.hpp"
#include "spike/type/pointer.hpp"

std::string_view filters[]{
    ".stq$",
    ".stqr$",
};

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = STQ2JSON_DESC " v" STQ2JSON_VERSION ", " STQ2JSON_COPYRIGHT
                            "Lukas Cone",
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

struct UnkStruct0 {
  es::PointerX64<char> soundPath;
  uint32 unk[8];
};

struct UnkStruct1 {};

struct Header {
  static constexpr uint32 ID = CompileFourCC("STQR");
  uint32 id;
  uint32 version;
  uint32 numUnk0;
  uint32 numUnk1;
  es::PointerX64<UnkStruct0> unk0;
  es::PointerX64<UnkStruct1> unk1;
};

void ProcessClass(Header &hdr) {
  char *root = reinterpret_cast<char *>(&hdr);

  hdr.unk0.Fixup(root);
  hdr.unk1.Fixup(root);

  UnkStruct0 *str0 = hdr.unk0;

  for (size_t i = 0; i < hdr.numUnk0; i++) {
    str0[i].soundPath.Fixup(root);
  }
}

void AppProcessFile(AppContext *ctx) {
  {
    Header hdr;
    ctx->GetType(hdr);

    if (hdr.id != Header::ID) {
      throw es::InvalidHeaderError(hdr.id);
    }

    if (hdr.version != 2) {
      throw es::InvalidVersionError(hdr.version);
    }
  }

  std::string buffer = ctx->GetBuffer();
  Header *hdr = reinterpret_cast<Header *>(buffer.data());
  ProcessClass(*hdr);

  UnkStruct0 *str0 = hdr->unk0;

  for (size_t i = 0; i < hdr->numUnk0; i++) {
    printinfo(static_cast<char *>(str0[i].soundPath));
  }
}
