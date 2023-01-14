/*  XFSConvert
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
#include "datas/pugiex.hpp"
#include "project.h"
#include "re_common.hpp"
#include "revil/xfs.hpp"
#include <algorithm>

struct XFS2XML : ReflectorBase<XFS2XML> {
  bool saveRTTI = true;
  bool saveData = true;
} settings;

REFLECT(CLASS(XFS2XML),
        MEMBERNAME(saveRTTI, "save-rtti", "r",
                   ReflDesc{"Save layout information."}),
        MEMBERNAME(saveData, "save-data", "d", ReflDesc{"Save data."}), );

static AppInfo_s appInfo{
    .header = XFSConvert_DESC " v" XFSConvert_VERSION ", " XFSConvert_COPYRIGHT
                              "Lukas Cone",
    .settings = reinterpret_cast<ReflectorFriend *>(&settings),
};

AppInfo_s *AppInitModule() { return &appInfo; }

void AppProcessFile(std::istream &stream, AppContext *ctx) {
  XFS xfs;
  xfs.Load(stream);

  auto &outStr = ctx->NewFile(ctx->workingFile.ChangeExtension(".xml"));

  pugi::xml_document doc;
  xfs.ToXML(doc);
  doc.save(outStr);
}
