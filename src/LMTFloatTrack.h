/*      Revil Format Library
        Copyright(C) 2017-2019 Lukas Cone

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
#include "LMTInternal.h"

struct FloatFrame {
  int data; // char numComponents, short frame, char pad (0xcd)
  Vector value;

  ES_FORCEINLINE short Frame() const { return static_cast<short>(data >> 8); }

  ES_FORCEINLINE void Frame(int input) {
    data &= 0xff0000ff;
    data |= (input << 8) & 0xffff00;
  }

  ES_FORCEINLINE uchar NumComponents() const {
    return static_cast<uchar>(data);
  }

  ES_FORCEINLINE void NumComponents(int input) {
    data &= 0xffffff00;
    data |= input & 0xff;
  }

  ES_FORCEINLINE void SwapEndian() {
    FByteswapper(data);
    FByteswapper(value);
  }

  Reflector::xmlNodePtr ToXML(pugi::xml_node &node) const;
  int FromXML(pugi::xml_node &node);
};

class LMTFloatTrack_internal : public LMTFloatTrack {
  virtual void _ToXML(pugi::xml_node &node, int groupID) const = 0;
  virtual void _FromXML(pugi::xml_node &node, int groupID) = 0;
  virtual void _Save(BinWritter *wr, LMTFixupStorage &storage) const = 0;
  virtual bool _Is64bit() const = 0;

public:
  typedef std::vector<FloatFrame, std::allocator_hybrid<FloatFrame>>
      FramesCollection;

protected:
  FramesCollection frames[4];

public:
  int GetNumGroups() const override { return 4; }

  int GetGroupTrackCount(int groupID) const override {
    return static_cast<int>(frames[groupID].size());
  }

  const FloatFrame *GetFrames(int groupID) const {
    return frames[groupID].data();
  }

  FloatFrame *GetFrames(int groupID) { return frames[groupID].data(); }

  virtual void SetNumFrames(int groupID, int newSize) {
    frames[groupID].resize(newSize);
  }

  int ToXML(pugi::xml_node &node, bool standAlone) const override;
  int FromXML(pugi::xml_node &node) override;
  int Save(BinWritter *wr) const;
};