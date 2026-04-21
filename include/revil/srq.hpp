/*  Revil Format Library
    Copyright(C) 2026 Lukas Cone

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
#include "settings.hpp"
#include "spike/io/bincore_fwd.hpp"
#include "spike/util/pugi_fwd.hpp"
#include <memory>

namespace revil {
struct SRQImpl;

enum class SRQVersion : uint32 {
  V_14 = 0xE,
};

class RE_EXTERN SRQ {
public:
  void Load(BinReaderRef_e rd);
  void ToXML(pugi::xml_node node) const;

  SRQ();
  ~SRQ();

private:
  std::unique_ptr<SRQImpl> pi;
};

void RE_EXTERN SRQFromXML(BinWritterRef wr, pugi::xml_node rootNode,
                          SRQVersion version);
} // namespace revil
