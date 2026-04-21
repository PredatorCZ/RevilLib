/*  Revil Format Library
    Copyright(C) 2020-2026 Lukas Cone

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

#pragma once
#include "platform.hpp"
#include "settings.hpp"
#include "spike/app_context.hpp"
#include "spike/except.hpp"
#include "spike/io/bincore_fwd.hpp"
#include <functional>
#include <set>

namespace revil {
struct ArcExtractContext : AppExtractContext {
  bool RequiresFolders() const override { return false; }
  void AddFolderPath(const std::string &) override {
    throw es::ImplementationError("Invalid call");
  }
  void GenerateFolders() override {
    throw es::ImplementationError("Invalid call");
  }
  NewTexelContext *NewImage(const std::string &,
                            NewTexelContextCreate) override {
    throw es::ImplementationError("Invalid call");
  }
};

void RE_EXTERN
EnumerateArchive(BinReaderRef_e rd, Platform platform, std::string_view title,
                 std::function<AppExtractContext *()> demandContext,
                 const std::set<uint32> &classFilter, bool filterIsBlackList = false);
size_t RE_EXTERN CompressZlib(std::string_view inBuffer, std::string &outBuffer, int windowSize, int level);
} // namespace revil
