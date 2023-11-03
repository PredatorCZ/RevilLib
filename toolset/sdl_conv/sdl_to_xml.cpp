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
#include "spike/io/binreader_stream.hpp"
#include "spike/util/pugiex.hpp"
#include <algorithm>

static AppInfo_s appInfo{
    .header = SDLConvert_DESC " v" SDLConvert_VERSION ", " SDLConvert_COPYRIGHT
                              "Lukas Cone",
};

AppInfo_s *AppInitModule() { return &appInfo; }

void AppProcessFile(AppContext *ctx) {
  SDL sdl;

  try {
    sdl.Load(ctx->GetStream());
  } catch (const es::InvalidHeaderError &r) {
    return;
  }

  auto &outStr = ctx->NewFile(ctx->workingFile.ChangeExtension(".xml")).str;

  pugi::xml_document doc;
  sdl.ToXML(doc);
  doc.save(outStr);
}
