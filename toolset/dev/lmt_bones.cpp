/*  LMTBones
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
#include "re_common.hpp"
#include "revil/lmt.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/io/binwritter.hpp"
#include "spike/io/fileinfo.hpp"
#include "spike/master_printer.hpp"
#include <set>

std::string_view filters[]{
    ".lmt$",
    ".bin$",
};

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = LMTBones_DESC " v" LMTBones_VERSION ", " LMTBones_COPYRIGHT
                            "Lukas Cone",
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

static std::map<std::string, std::map<size_t, uint16>> REPORT;

void AppProcessFile(AppContext *ctx) {
  revil::LMT lmt;
  lmt.Load(ctx->GetStream());
  uni::MotionsConst motions = lmt;
  std::map<size_t, uint16> bones;

  for (auto m : *motions) {
    if (!m) {
      continue;
    }
    for (auto t : *m) {
      auto tm = static_cast<const LMTTrack *>(t.get());
      if (tm->BoneType() > 2 || tm->BoneType() == 1) {
        bones[tm->BoneIndex()] = tm->BoneType();
      }
    }
  }

  if (bones.size()) {
    static std::mutex mtx;
    std::lock_guard<std::mutex> lg(mtx);
    REPORT.emplace(ctx->workingFile.GetFilename(), bones);
  }
}

void AppFinishContext() {
  BinWritter wr("lmt_bones.txt");
  auto &str = wr.BaseStream();
  for (auto &[name, bones] : REPORT) {
    str << name << ":";

    for (auto &[id, type] : bones) {
      str << '\t' << id << '(' << type << "),";
    }

    str << '\n';
  }
}
