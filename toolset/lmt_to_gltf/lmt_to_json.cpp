/*  LMT2JSON
    Copyright(C) 2022-2023 Lukas Cone

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

#include "nlohmann/json.hpp"
#include "project.h"
#include "re_common.hpp"
#include "revil/lmt.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/io/binwritter_stream.hpp"
#include "spike/io/fileinfo.hpp"

std::string_view filters[]{
    ".lmt$",
};

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = LMT2JSON_DESC " v" LMT2JSON_VERSION ", " LMT2JSON_COPYRIGHT
                            "Lukas Cone",
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

void AppProcessFile(AppContext *ctx) {
  LMT lmt;
  lmt.Load(ctx->GetStream());
  uni::MotionsConst motion = lmt;
  std::string name(ctx->workingFile.GetFilename());
  nlohmann::json main(nlohmann::json::object());

  for (size_t motionIndex = 0; auto m : *motion) {
    if (!m) {
      motionIndex++;
      continue;
    }

    auto tm = static_cast<const LMTAnimation *>(m.get());
    auto events = tm->Events();

    if (!events) {
      motionIndex++;
      continue;
    }

    std::string animName = name + "[" + std::to_string(motionIndex) + "]";
    auto eventsVar = events->Get();
    auto eventsV1 = std::get<const LMTAnimationEventV1 *>(eventsVar);
    nlohmann::json groups(nlohmann::json::object());

    for (size_t g = 0; g < events->GetNumGroups(); g++) {
      auto collection = eventsV1->GetEvents(g);

      if (collection.size() > 0) {
        groups.emplace(std::to_string(g), collection);
      }
    }

    if (groups.size() > 0) {
      main.emplace(animName, std::move(groups));
    }

    motionIndex++;
  }

  if (main.size() > 0) {
    BinWritterRef wr(
        ctx->NewFile(ctx->workingFile.ChangeExtension(".json")).str);
    wr.BaseStream() << main;
  }
}
