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

using LMTTracks = uni::PolyVectorList<uni::MotionTrack, LMTTrack>;

class LMTAnimation_internal : public LMTAnimation, public LMTTracks {
  virtual void ReflectToXML(pugi::xml_node node) const = 0;
  virtual void ReflectFromXML(pugi::xml_node node) = 0;
  virtual void SaveInternal(BinWritterRef wr,
                            LMTFixupStorage &storage) const = 0;
  virtual bool Is64bit() const = 0;

public:
  using MasterBufferPtr = uni::Element<std::string>;
  using LMTEventsPtr = std::unique_ptr<LMTAnimationEvent>;
  using LMTFloatTrackPtr = std::unique_ptr<LMTFloatTrack>;
  using LMTTrackPtr = typename LMTTracks::class_type;

  MasterBufferPtr masterBuffer;
  LMTEventsPtr events;
  LMTFloatTrackPtr floatTracks;

  virtual LMTTrackPtr CreateTrack() const = 0;
  virtual LMTEventsPtr CreateEvents() const = 0;
  virtual LMTFloatTrackPtr CreateFloatTracks() const = 0;
  virtual std::vector<uint64> GetPtrValues() const = 0;

  void Save(pugi::xml_node node, bool standAlone = false) const override;
  void Save(BinWritterRef wr, bool standAlone = true) const override;
  void Load(pugi::xml_node node) override;

  static Ptr Load(BinReaderRef_e rd, LMTConstructorPropertiesBase expected);

  uni::MotionTracksConst Tracks() const override {
    return uni::MotionTracksConst(this, false);
  }

  std::string Name() const override { return ""; }
  void FrameRate(uint32 fps) const override;
  uint32 FrameRate() const override;
  float Duration() const override;
  MotionType_e MotionType() const override { return MotionType_e::Relative; }
};
