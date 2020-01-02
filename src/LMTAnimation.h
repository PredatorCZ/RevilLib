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

class LMTAnimation_internal : public LMTAnimation {
  static const int MTMI = CompileFourCC("MTMI");
  virtual void _ToXML(pugi::xml_node &node) const = 0;
  virtual void _FromXML(pugi::xml_node &node) = 0;
  virtual void _Save(BinWritter *wr, LMTFixupStorage &storage) const = 0;
  virtual bool _Is64bit() const = 0;

public:
  typedef std::unique_ptr<char, std::deleter_hybrid_c> MasterBufferPtr;
  typedef std::unique_ptr<LMTAnimationEvent> LMTEventsPtr;
  typedef std::unique_ptr<LMTTrack> LMTTrackPtr;
  typedef std::unique_ptr<LMTFloatTrack> LMTFloatTrackPtr;

  MasterBufferPtr masterBuffer;
  std::vector<LMTTrackPtr> tracks;
  LMTEventsPtr events;
  LMTFloatTrackPtr floatTracks;

  LMTAnimation_internal() : masterBuffer(nullptr) {}

  LMTConstructorPropertiesBase &_Props() { return props; }

  virtual LMTTrackPtr CreateTrack() const = 0;
  virtual LMTEventsPtr CreateEvents() const = 0;
  virtual LMTFloatTrackPtr CreateFloatTracks() const = 0;
  virtual std::vector<uint64> GetPtrValues() const = 0;

  int Save(BinWritter *wr, bool standAlone = true) const;
  static int Load(BinReader *rd, LMTConstructorPropertiesBase expected,
                  LMTAnimation_internal *&out);

  const int NumTracks() const override {
    return static_cast<int>(tracks.size());
  };

  const LMTTrack *Track(int id) const override { return tracks[id].get(); }

  int FromXML(pugi::xml_node &node) override;
  int ToXML(pugi::xml_node &node, bool standAlone) const override;
};
