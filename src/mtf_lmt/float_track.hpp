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
#include "internal.hpp"

struct FloatFrame {
  uint32 data; // uint8 numComponents, uint16 frame, uint8 pad (0xcd)
  Vector value;

  int16 Frame() const { return static_cast<int16>(data >> 8); }

  void Frame(int16 input) {
    data &= 0xff0000ff;
    data |= (input << 8) & 0xffff00;
  }

  uint8 NumComponents() const { return static_cast<uint8>(data); }

  void NumComponents(int16 input) {
    data &= 0xffffff00;
    data |= input & 0xff;
  }

  void Save(pugi::xml_node node) const;
  void Load(pugi::xml_node node);
};

class LMTFloatTrack_internal : public LMTFloatTrack {
public:
  size_t GetNumGroups() const override { return 4; }
  virtual const FloatFrame *GetFrames(size_t groupID) const = 0;
  virtual FloatFrame *GetFrames(size_t groupID) = 0;
};

enum class FloatTrackComponentRemap : uint8 {
  NONE, X_COMP, Y_COMP, Z_COMP
};
