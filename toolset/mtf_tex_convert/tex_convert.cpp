/*  MTFTEXConvert
    Copyright(C) 2021-2025 Lukas Cone

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
#include "re_common.hpp"
#include "revil/arc.hpp"
#include "revil/hashreg.hpp"
#include "revil/tex.hpp"
#include "spike/io/binreader_stream.hpp"
#include <spanstream>

std::string_view filters[]{
    ".tex$",
    ".arc$",
};

struct TEXConvert : ReflectorBase<TEXConvert> {
  Platform platformOverride = Platform::Auto;
} settings;

REFLECT(CLASS(TEXConvert),
        MEMBERNAME(platformOverride, "platform", "p",
                   ReflDesc{"Set platform for correct texture handling."}));

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = MTFTEXConvert_DESC " v" MTFTEXConvert_VERSION
                                 ", " MTFTEXConvert_COPYRIGHT "Lukas Cone",
    .settings = reinterpret_cast<ReflectorFriend *>(&settings),
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

void Convert(std::istream &str, const std::string &path, AppContext *ctx) {
  TEX tex;
  tex.Load(str, settings.platformOverride);

  std::string fullPath(ctx->workingFile.GetFolder());
  fullPath.append(path);
  fullPath.append(".glb");

  auto tctx = ctx->NewImage(tex.ctx, &fullPath);

  if (tex.ctx.numFaces > 0) {
    for (uint16 f = 0; f < tex.ctx.numFaces; f++) {
      for (uint8 m = 0; m < tex.ctx.numMipmaps; m++) {
        TexelInputLayout layout{
            .mipMap = m,
            .face = CubemapFace(f + 1),
        };
        tctx->SendRasterData(tex.buffer.data() +
                                 tex.offsets.at(f * tex.ctx.numMipmaps + m),
                             layout);
      }
    }
  } else {
    for (uint8 m = 0; m < tex.ctx.numMipmaps; m++) {
      TexelInputLayout layout{
          .mipMap = m,
      };
      tctx->SendRasterData(tex.buffer.data() + tex.offsets.at(m), layout);
    }
  }
}

struct ExtractContext : ArcExtractContext {
  AppContext *parentCtx;
  std::string curFile;

  void NewFile(const std::string &path) override {
    curFile = AFileInfo(path).GetFullPathNoExt();
  }
  void SendData(std::string_view data) override {
    std::ispanstream spstr(std::span<const char>(data.data(), data.size()));
    Convert(spstr, curFile, parentCtx);
  }
};

void AppProcessFile(AppContext *ctx) {
  if (ctx->workingFile.GetExtension() == ".arc") {
    static const std::set<uint32> filter{
        MTHashV1("rTexture"),
        MTHashV2("rTexture"),
    };

    ExtractContext ectx;
    ectx.parentCtx = ctx;

    EnumerateArchive(
        ctx->GetStream(), settings.platformOverride, "lp",
        [&] { return &ectx; }, filter);
    return;
  }

  Convert(ctx->GetStream(), std::string(ctx->workingFile.GetFilenameExt()),
          ctx);
}
