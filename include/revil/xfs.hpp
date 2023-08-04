/*  Revil Format Library
    Copyright(C) 2017-2023 Lukas Cone

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
#include "spike/io/bincore_fwd.hpp"
#include "spike/util/pugi_fwd.hpp"
#include "settings.hpp"
#include <memory>

namespace revil {
class XFSImpl;

class RE_EXTERN XFS {
public:
  void Load(BinReaderRef_e rd);
  void ToXML(pugi::xml_node node) const;
  void RTTIToXML(pugi::xml_node node) const;

  XFS();
  ~XFS();

private:
  std::unique_ptr<XFSImpl> pi;
};
} // namespace revil
