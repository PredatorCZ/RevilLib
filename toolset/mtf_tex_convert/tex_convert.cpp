/*  MTFTEXConvert
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

#include "datas/binreader_stream.hpp"
#include "datas/binwritter_stream.hpp"
#include "datas/fileinfo.hpp"
#include "project.h"
#include "re_common.hpp"
#include "revil/tex.hpp"

std::string_view filters[]{
    ".tex$",
};

struct TEXConvert : ReflectorBase<TEXConvert>, Tex2DdsSettings {
} settings;

REFLECT(CLASS(TEXConvert),
        MEMBERNAME(
            convertIntoLegacy, "legacy-dds", "l",
            ReflDesc{"Tries to convert texture into legacy (DX9) DDS format."}),
        MEMBERNAME(convertIntoLegacyNonCannon, "force-legacy-dds", "f",
                   ReflDesc{"Will try to convert some matching formats from "
                            "DX10 to DX9, for example: RG88 to AL88."}),
        MEMBERNAME(noMips, "largest-mipmap-only", "m",
                   ReflDesc{"Will try to extract only highest mipmap."}),
        MEMBERNAME(platformOverride, "platform", "p",
                   ReflDesc{"Set platform for correct texture handling."}));

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = MTFTEXConvert_DESC " v" MTFTEXConvert_VERSION
                                 ", " MTFTEXConvert_COPYRIGHT "Lukas Cone",
    .settings = reinterpret_cast<ReflectorFriend *>(&settings),
    .filters = filters,
};

AppInfo_s *AppInitModule() {
  return &appInfo;
}

void AppProcessFile(AppContext *ctx) {
  TEX tex;
  tex.Load(ctx->GetStream(), settings.platformOverride);

  AFileInfo fleInfo0(ctx->workingFile);
  BinWritterRef wr(ctx->NewFile(fleInfo0.ChangeExtension(".dds")).str);
  tex.SaveAsDDS(wr, settings);
}
