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
#include "internal.hpp"
#include "spike/reflect/reflector.hpp"
#include <span>

class AnimEvent {
public:
  uint32 runEventBit;
  uint32 numFrames;

  void Save(pugi::xml_node node) const;
  void Load(pugi::xml_node node);
  void SwapEndian();
};

class LMTAnimationEventInterface : public LMTAnimationEvent,
                                   public LMTAnimationEventV1 {
public:
  float frameRate = 60.f;
  void Save(BinWritterRef wr) const;
  void SaveBuffer(BinWritterRef wr, LMTFixupStorage &fixups) const;
  void Save(pugi::xml_node node) const;
};

MAKE_ENUM(ENUMSCOPE(class EventFrameV2DataType
                    : uint16, EventFrameV2DataType),
          EMEMBER(Int8), EMEMBER(Int32), EMEMBER(Float), EMEMBERVAL(Bool, 4));
MAKE_ENUM(ENUMSCOPE(class EventFrameV2Type
                    : uint16, EventFrameV2Type),
          EMEMBER(Scalar), EMEMBER(Scalar2), EMEMBER(Scalar3),
          EMEMBER(PackedFloat), EMEMBERVAL(PackedBitFlags, 5),
          EMEMBER(PackedInt));

struct AnimEventFrameV2 {
  union {
    Vector fdata;
    IVector idata;
  };
  float frame;
  EventFrameV2Type type;
  EventFrameV2DataType dataType;

  AnimEventFrameV2() : idata(), frame() {}

  void Save(pugi::xml_node node) const;
  void Load(pugi::xml_node node);
};

struct AnimEventV2 {
  using FramesPtr = esPointerX64<AnimEventFrameV2>;

  FramesPtr frames;
  uint64 numFrames;
  uint32 eventHash;
  EventFrameV2DataType dataType;
};

struct AnimEventGroupV2 {
  using EventPtr = esPointerX64<AnimEventV2>;

  EventPtr events;
  uint64 numEvents;
  uint32 groupHash;
};

struct AnimEventsHeaderV2 {
  using GroupPtr = esPointerX64<AnimEventGroupV2>;

  GroupPtr eventGroups;
  uint64 numGroups;
  uint32 totalNumEvents;
  uint32 totalNumEventFrames;
  float numFrames;
  float loopFrame;
  uint32 null00;
  uint32 collectionHash;
};
