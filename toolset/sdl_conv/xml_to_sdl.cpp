/*  SDLConvert
    Copyright(C) 2023 Lukas Cone

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
#include "revil/sdl.hpp"
#include "spike/io/binwritter_stream.hpp"
#include "spike/util/pugiex.hpp"

std::string_view filters[]{
    ".xml$",
};

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = SDLConvert_DESC " v" SDLConvert_VERSION ", " SDLConvert_COPYRIGHT
                              "Lukas Cone",
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

  auto &str = ctx->NewFile(ctx->workingFile.ChangeExtension2("sdl")).str;
  revil::SDLFromXML(str, doc);
}
