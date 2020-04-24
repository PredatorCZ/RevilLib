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
#include "LMTInternal.h"

typedef uni::VectorList<uni::MotionTrack, LMTTrack> LMTTracks;

class LMTAnimation_internal : public LMTAnimation, public LMTTracks {
  static const uint32 MTMI = CompileFourCC("MTMI");
  virtual void _ToXML(pugi::xml_node &node) const = 0;
  virtual void _FromXML(pugi::xml_node &node) = 0;
  virtual void _Save(BinWritterRef wr, LMTFixupStorage &storage) const = 0;
  virtual bool _Is64bit() const = 0;

public:
  typedef std::unique_ptr<char, es::deleter_hybrid_c> MasterBufferPtr;
  typedef std::unique_ptr<LMTAnimationEvent> LMTEventsPtr;
  typedef std::unique_ptr<LMTFloatTrack> LMTFloatTrackPtr;
  typedef typename LMTTracks::pointer_class_type LMTTrackPtr;

  MasterBufferPtr masterBuffer;
  LMTEventsPtr events;
  LMTFloatTrackPtr floatTracks;

  LMTAnimation_internal() : masterBuffer(nullptr) {}

  LMTConstructorPropertiesBase &_Props() { return props; }

  virtual LMTTrackPtr CreateTrack() const = 0;
  virtual LMTEventsPtr CreateEvents() const = 0;
  virtual LMTFloatTrackPtr CreateFloatTracks() const = 0;
  virtual std::vector<uint64> GetPtrValues() const = 0;

  int Save(BinWritterRef wr, bool standAlone = true) const;
  static int Load(BinReaderRef rd, LMTConstructorPropertiesBase expected,
                  LMTAnimation_internal *&out);

  uni::MotionTracksConst Tracks() const override {
    return uni::MotionTracksConst(this, false);
  }

  std::string Name() const override { return ""; }
  void FrameRate(uint32 fps) override;
  uint32 FrameRate() const override;
  float Duration() const override;
  MotionType_e MotionType() const override { return MotionType_e::Relative; }

  int FromXML(pugi::xml_node &node) override;
  int ToXML(pugi::xml_node &node, bool standAlone) const override;
};
