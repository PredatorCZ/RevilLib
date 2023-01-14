/*  TEXConvert
    Copyright(C) 2020-2022 Lukas Cone

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
#include "formats/DDS.hpp"
#include "project.h"
#include <algorithm>

std::string_view filters[]{
    ".tex$",
};

struct TEXConvert : ReflectorBase<TEXConvert> {
  bool legacyDDS = true;
  bool forceLegacyDDS = false;
  bool largestMipmap = true;
} settings;

REFLECT(CLASS(TEXConvert),
        MEMBERNAME(legacyDDS, "legacy-dds", "l",
                   ReflDesc{
                       "Tries to convert texture into legacy (DX9) DDS format.",
                       ""}),
        MEMBERNAME(forceLegacyDDS, "force-legacy-dds", "f",
                   ReflDesc{
                       "Will try to convert some matching formats from DX10 "
                       "to DX9, for example: RG88 to AL88.",
                       ""}),
        MEMBERNAME(largestMipmap, "largest-mipmap-only", "m",
                   ReflDesc{"Will try to extract only highest mipmap.", ""}), );

AppInfo_s appInfo{
    .filteredLoad = true,
    .header = RETEXConvert_DESC " v" RETEXConvert_VERSION
                                ", " RETEXConvert_COPYRIGHT "Lukas Cone",
    .settings = reinterpret_cast<ReflectorFriend *>(&settings),
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

struct RETEXMip {
  char *offset;
  uint32 pad;
  uint32 unk;
  uint32 size;
};

struct RETEX {
  static constexpr uint32 ID = CompileFourCC("TEX\0");
  uint32 id, version;
  uint16 width, height,
      depth; // vector field
  uint8 numMips;
  uint8 numArrays;
  DXGI_FORMAT format;
  int32 unk01; // -1
  uint32 unk;  // 4 = cubemap
  uint32 flags;

  const RETEXMip *Mips() const {
    return reinterpret_cast<const RETEXMip *>(this + 1);
  }

  void Fixup() {
    char *root = reinterpret_cast<char *>(this);
    RETEXMip *mips = reinterpret_cast<RETEXMip *>(this + 1);
    uint32 numTotalMips = numMips * numArrays;

    for (uint32 t = 0; t < numTotalMips; t++) {
      mips[t].offset = root + reinterpret_cast<uintptr_t>(mips[t].offset);
    }
  }
};

void AppProcessFile(AppContext *ctx) {
  uint32 id;
  ctx->GetType(id);

  if (id != RETEX::ID) {
    throw es::InvalidHeaderError(id);
  }

  std::string buffer = ctx->GetBuffer();
  RETEX *tex = reinterpret_cast<RETEX *>(&buffer[0]);
  tex->Fixup();

  BinWritterRef wr(ctx->NewFile(ctx->workingFile.ChangeExtension(".dds")));

  DDS ddtex = {};
  ddtex = DDSFormat_DX10;
  ddtex.dxgiFormat = tex->format;
  ddtex.width = tex->width;
  ddtex.height = tex->height;

  if (tex->depth > 1) {
    ddtex.depth = tex->depth;
    ddtex.flags += DDS::Flags_Depth;
    ddtex.caps01 += DDS_HeaderEnd::Caps01Flags_Volume;
  } else if (tex->unk != 4) {
    ddtex.arraySize = tex->numArrays;
  } else {
    ddtex.caps01 = decltype(ddtex.caps01)(
        DDS::Caps01Flags_CubeMap, DDS::Caps01Flags_CubeMap_NegativeX,
        DDS::Caps01Flags_CubeMap_NegativeY, DDS::Caps01Flags_CubeMap_NegativeZ,
        DDS::Caps01Flags_CubeMap_PositiveX, DDS::Caps01Flags_CubeMap_PositiveY,
        DDS::Caps01Flags_CubeMap_PositiveZ);
  }

  ddtex.NumMipmaps(settings.largestMipmap ? 1 : tex->numMips);

  const uint32 sizetoWrite = !settings.legacyDDS || ddtex.arraySize > 1 ||
                                     ddtex.ToLegacy(settings.forceLegacyDDS)
                                 ? ddtex.DDS_SIZE
                                 : ddtex.LEGACY_SIZE;

  if (settings.legacyDDS && sizetoWrite == ddtex.DDS_SIZE) {
    printwarning("Couldn't convert DX10 dds to legacy.")
  }

  wr.WriteBuffer(reinterpret_cast<const char *>(&ddtex), sizetoWrite);

  const RETEXMip *mips = tex->Mips();
  const uint32 mipPerArray = tex->numMips;

  for (uint32 a = 0; a < tex->numArrays; a++) {
    for (uint32 m = 0; m < ddtex.mipMapCount; m++) {
      const RETEXMip &cMip = mips[m + mipPerArray * a];
      wr.WriteBuffer(cMip.offset, cMip.size * tex->depth);
    }
  }
}
