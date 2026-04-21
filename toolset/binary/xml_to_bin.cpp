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
#include "revil/hashreg.hpp"
#include "revil/sdl.hpp"
#include "spike/io/binwritter_stream.hpp"
#include "spike/master_printer.hpp"
#include "spike/util/pugiex.hpp"

std::string_view filters[]{
    ".xml$",
};

struct MTFBinaryConvertXML : ReflectorBase<MTFBinaryConvertXML> {
  std::string title;
  Platform platform = Platform::Auto;
} settings;

REFLECT(CLASS(MTFBinaryConvertXML),
        MEMBER(title, "t", ReflDesc{"Set title for correct archive handling."}),
        MEMBER(platform, "p",
               ReflDesc{"Set platform for correct archive handling."}));

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = MTFBinaryConvert_DESC
    " v" MTFBinaryConvert_VERSION ", " MTFBinaryConvert_COPYRIGHT "Lukas Cone",
    .settings = reinterpret_cast<ReflectorFriend *>(&settings),
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

void AppProcessFile(AppContext *ctx) {
  pugi::xml_document doc;

  if (auto result = doc.load(ctx->GetStream()); !result) {
    throw std::runtime_error(
        std::string("Couldn't load XML [") +
        GetReflectedEnum<XMLError>()->names[result.status] + "] at offset " +
        std::to_string(result.offset));
  }

  auto rootNode = doc.child("class");

  if (rootNode.empty()) {
    throw std::runtime_error("Cannot find class root node, got: " +
                             std::string(rootNode.name()));
  }

  pugi::xml_attribute rootAttr = rootNode.attribute("type");

  if (rootAttr.empty()) {
    throw es::RuntimeError("Cannot find class root node type attribute");
  }

  const uint32 classHash = JenkinsHash_(rootAttr.as_string());
  const std::string outPath(ctx->workingFile.GetFullPathNoExt());
  std::ostream *ostr = nullptr;
  const TitleSupport *supp =
      revil::GetTitleSupport(settings.title, settings.platform);

  switch (classHash) {
  case JenkinsHash_("rSchedulerXml"):
    ostr = &ctx->NewFile(outPath).str;
    revil::SDLFromXML(*ostr, doc, SDLVersion(supp->sdlVersion));
    break;
  default:
    PrintError("Unknown xml class to process: ", rootAttr.as_string());
    break;
  }
}
