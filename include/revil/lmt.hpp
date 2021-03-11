/*  Revil Format Library
    Copyright(C) 2017-2020 Lukas Cone

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
#include "datas/string_view.hpp"
#include "mot.hpp"

namespace revil {

enum class LMTExportType : uint8 {
  FullBinary,      // Propper LMT format
  FullXML,         // Single XML format
  LinkedXML,       // XML format with linked XML files per animation
  BinaryLinkedXML, // XML format with linked binary files per animation
};

struct LMTExportSettings {
  LMTExportType type = LMTExportType::FullBinary;
  LMTArchType arch = LMTArchType::Auto;
  LMTVersion version = LMTVersion::Auto;
  bool swapEndian = false;
};

class LMTImpl;

class RE_EXTERN LMT {
public:
  LMT();
  ~LMT();
  LMTVersion Version() const;
  void Version(LMTVersion version, LMTArchType arch);
  LMTArchType Architecture() const;
  auto CreateAnimation() const;

  LMTAnimation *AppendAnimation();
  void AppendAnimation(LMTAnimation *ani);
  void InsertAnimation(LMTAnimation *ani, size_t at, bool replace = false);

  void Load(BinReaderRef rd);
  void Load(const std::string &fileName, LMTImportOverrides overrides = {});
  void Load(pugi::xml_node node, es::string_view outPath,
            LMTImportOverrides overrides = {});
  void Save(BinWritterRef wr) const;
  void Save(const std::string &fileName, LMTExportSettings settings = {}) const;
  void Save(pugi::xml_node node, es::string_view outPath,
            LMTExportSettings settings = {}) const;

  operator uni::MotionsConst() const;

private:
  std::unique_ptr<LMTImpl> pi;
};

// TODO:
/*
add header sanitizers

low priority:
conversion system << super low
encoder, exporting utility
*/
} // namespace revil
