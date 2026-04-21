/*  MTFBinaryConvert
    Copyright(C) 2023-2026 Lukas Cone

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
#include "revil/cdf.hpp"
#include "revil/hashreg.hpp"
#include "revil/hit.hpp"
#include "revil/oba.hpp"
#include "revil/osf.hpp"
#include "revil/scs.hpp"
#include "revil/sdl.hpp"
#include "revil/sds.hpp"
#include "revil/srd.hpp"
#include "revil/srq.hpp"
#include "revil/stq.hpp"
#include "revil/xfs.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/io/binwritter_stream.hpp"
#include "spike/master_printer.hpp"
#include "spike/util/pugiex.hpp"
#include <cassert>
#include <spanstream>
#include <sstream>

struct MTFBinaryConvert : ReflectorBase<MTFBinaryConvert> {
  std::string title;
  Platform platform = Platform::Auto;
} settings;

REFLECT(CLASS(MTFBinaryConvert),
        MEMBER(title, "t", ReflDesc{"Set title for correct archive handling."}),
        MEMBER(platform, "p",
               ReflDesc{"Set platform for correct archive handling."}));

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = MTFBinaryConvert_DESC
    " v" MTFBinaryConvert_VERSION ", " MTFBinaryConvert_COPYRIGHT "Lukas Cone",
    .settings = reinterpret_cast<ReflectorFriend *>(&settings),
};

AppInfo_s *AppInitModule() { return &appInfo; }

template <class C>
void Convert(std::istream &str, const std::string &path, AppContext *ctx) {
  C bin;
  bin.Load(str);

  std::string fullPath(ctx->workingFile.GetFolder());
  fullPath.append(path);
  fullPath.append(".xml");

  auto &outStr = ctx->NewFile(fullPath).str;

  pugi::xml_document doc;
  bin.ToXML(doc);
  doc.save(outStr);
}

void ConvertSDL(std::istream &str, const std::string &path, AppContext *ctx) {
  SDL sdl;

  try {
    sdl.Load(str);
  } catch (const es::InvalidHeaderError &r) {
    return;
  }

  std::string fullPath(ctx->workingFile.GetFolder());
  fullPath.append(path);
  fullPath.append(".xml");

  auto &outStr = ctx->NewFile(fullPath).str;

  pugi::xml_document doc;
  sdl.ToXML(doc);
  doc.save(outStr);
  /*
    std::stringstream sstr;
    doc.save(sstr);

    std::string buff0 = std::move(sstr).str();
    AFileInfo finf(fullPath);

    auto &ostr = ctx->NewFile(finf.ChangeExtension(".xml.sdl")).str;
    revil::SDLFromXML(ostr, doc, SDLVersion::V_20);
    sstr = {};
    revil::SDLFromXML(sstr, doc, SDLVersion::V_20);

    SDL sdl2;
    sdl2.Load(sstr);
    auto &str2 =
        ctx->NewFile(finf.ChangeExtension(".xml.sdl.xml")).str;
    pugi::xml_document doc2;
    sdl2.ToXML(doc2);
    doc2.save(str2);
    sstr = {};
    doc2.save(sstr);

    std::string buff1 = std::move(sstr).str();

    if(buff0 != buff1) {
      //throw es::RuntimeError("Comparison failed");
      PrintError("Comparison failed: ", fullPath);
    }*/
}

void Detect(std::istream &str, const std::string &curFile,
            AppContext *parentCtx) {
  uint32 id;
  str.read(reinterpret_cast<char *>(&id), 4);
  str.seekg(0);

  switch (id) {
  case CompileFourCC("SDL"):
  case CompileFourCC("\0LDS"):
    ConvertSDL(str, curFile, parentCtx);
    return;
  case CompileFourCC("OSF"):
    Convert<OSF>(str, curFile, parentCtx);
    return;
  case CompileFourCC("HIT "):
    Convert<HIT>(str, curFile, parentCtx);
    return;
  case CompileFourCC("OBJA"):
    Convert<OBA>(str, curFile, parentCtx);
    return;
  case CompileFourCC("SREQ"):
    Convert<SRQ>(str, curFile, parentCtx);
    return;
  case CompileFourCC("DNRS"):
    Convert<SRD>(str, curFile, parentCtx);
    return;
  case CompileFourCC("CDF\3"):
    Convert<CDF>(str, curFile, parentCtx);
    return;
  case CompileFourCC("SCST"):
    Convert<SCS>(str, curFile, parentCtx);
    return;
  case CompileFourCC("SDST"):
    Convert<SDS>(str, curFile, parentCtx);
    return;
  case CompileFourCC("STRQ"):
    Convert<STQ>(str, curFile, parentCtx);
    return;
  case CompileFourCC("XFS"):
  case CompileFourCC("\0SFX"):
    Convert<XFS>(str, curFile, parentCtx);
    return;

  case CompileFourCC("ARCS"):
  case CompileFourCC("ESL"):
  case CompileFourCC("EFA"):
  case CompileFourCC("RRD "):
  case CompileFourCC("WED "):
  case CompileFourCC("EFS"):
  case CompileFourCC("FCA"):
  case CompileFourCC("FCP"):
    return;
  }

  PrintWarning("Undetected file: ", curFile);
}

struct ExtractContext : ArcExtractContext {
  AppContext *parentCtx;
  std::string curFile;

  void NewFile(const std::string &path) override { curFile = path; }
  void SendData(std::string_view data) override {
    std::ispanstream spstr(std::span<const char>(data.data(), data.size()));
    Detect(spstr, curFile, parentCtx);
  }
};

void AppProcessFile(AppContext *ctx) {
  if (ctx->workingFile.GetExtension() == ".arc") {
    static const std::set<uint32> filter{
        MTHashV1("rModel"),
        MTHashV2("rModel"),
        MTHashV1("rTexture"),
        MTHashV2("rTexture"),
        MTHashV1("rShader"),
        MTHashV2("rShader"),
        MTHashV2("rShaderPackage"),
        MTHashV1("rSoundPackage"),
        MTHashV2("rSoundPackage"),
        MTHashV1("rMotionList"),
        MTHashV2("rMotionList"),
        MTHashV1("rHkCollision"),
        MTHashV2("rHkCollision"),
        MTHashV2("rMotionList"),
        MTHashV1("rCollision"),
        MTHashV2("rCollision"),
        MTHashV1("rSoundSourceMusic"),
        MTHashV2("rSoundSourceMusic"),
    };

    ExtractContext ectx;
    ectx.parentCtx = ctx;

    EnumerateArchive(
        ctx->GetStream(), settings.platform, settings.title,
        [&] { return &ectx; }, filter, true);
    return;
  }

  Detect(ctx->GetStream(), std::string(ctx->workingFile.GetFilenameExt()), ctx);
}
