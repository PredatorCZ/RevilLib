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

class AnimEvent {
public:
  uint32 runEventBit;
  uint32 numFrames;

  pugi::xml_node ToXML(pugi::xml_node &node) const;
  int FromXML(pugi::xml_node &node);
  void SwapEndian();
};

template <template <class C> class PtrType>
struct AnimEvents : ReflectorInterface<AnimEvents<PtrType>> {
  uint16 eventRemaps[32];
  uint32 numEvents;
  PtrType<AnimEvent> events;

  void SwapEndian();
  void Fixup(char *masterBuffer, bool swapEndian);
};

class LMTAnimationEventV1_Internal : public LMTAnimationEventV1 {
  virtual void _ToXML(pugi::xml_node &node, uint32 groupID) const = 0;
  virtual void _FromXML(pugi::xml_node &node, uint32 groupID) = 0;
  virtual void _Save(BinWritterRef wr, LMTFixupStorage &fixups) const = 0;
  virtual bool _Is64bit() const = 0;

public:
  typedef std::vector<AnimEvent, es::allocator_hybrid<AnimEvent>>
      EventsCollection;

  virtual const EventsCollection &GetEvents(uint32 groupID) const = 0;
  virtual EventsCollection &GetEvents(uint32 groupID) = 0;
  virtual const uint16 *GetRemaps(uint32 groupID) const = 0;
  virtual void SetNumEvents(uint32 groupID, uint32 newSize) = 0;

  int ToXML(pugi::xml_node &node, bool standAlone) const override;
  int FromXML(pugi::xml_node &node) override;
  int Save(BinWritterRef wr) const;
  int SaveBuffer(BinWritterRef wr, LMTFixupStorage &fixups) const;
  EventCollection GetEvents(uint32 groupID, uint32 eventID) const override;
};

REFLECTOR_CREATE(EventFrameV2DataType, ENUM, 2, CLASS, 16, Int8, Int32, Float,
                 Bool = 4);
REFLECTOR_CREATE(EventFrameV2Type, ENUM, 2, CLASS, 16, Scalar, Scalar2, Scalar3,
                 PackedFloat, PackedBitFlags = 5, PackedInt);

struct AnimEventFrameV2 : ReflectorInterface<AnimEventFrameV2> {
  union {
    Vector fdata;
    IVector idata;
  };
  float frame;
  EventFrameV2Type type;
  EventFrameV2DataType dataType;

  AnimEventFrameV2() : idata(), frame(0.0f) {}

  int ToXML(pugi::xml_node &node) const;
  int FromXML(pugi::xml_node &node);
  void SwapEndian();
};

class LMTAnimationEventV2Event {
public:
  typedef std::vector<AnimEventFrameV2, es::allocator_hybrid<AnimEventFrameV2>>
      FramesCollection;

  FramesCollection frames;

  virtual uint32 GetHash() const = 0;
  virtual void SetHash(uint32 nHash) = 0;
  virtual void _Save(BinWritterRef wr, LMTFixupStorage &storage) const = 0;

  static LMTAnimationEventV2Event *Create();
  static LMTAnimationEventV2Event *FromXML(pugi::xml_node &node);

  virtual ~LMTAnimationEventV2Event() {}
};

class LMTAnimationEventV2Group {
public:
  typedef std::unique_ptr<LMTAnimationEventV2Event> EventPtr;
  typedef std::vector<EventPtr> EventsCollection;

  EventsCollection events;

  virtual uint32 GetHash() const = 0;
  virtual void SetHash(uint32 nHash) = 0;
  virtual void _Save(BinWritterRef wr, LMTFixupStorage &storage) const = 0;

  static LMTAnimationEventV2Group *Create();
  static LMTAnimationEventV2Group *FromXML(pugi::xml_node &node);

  virtual ~LMTAnimationEventV2Group() {}
};

class LMTAnimationEventV2_Internal : public LMTAnimationEventV2 {
  int ToXML(pugi::xml_node &node, bool standAlone) const override;
  int FromXML(pugi::xml_node &node) override;
  virtual void _Save(BinWritterRef wr, LMTFixupStorage &storage) const = 0;

public:
  typedef std::unique_ptr<LMTAnimationEventV2Group> GroupPtr;
  typedef std::vector<GroupPtr> GroupsCollection;

  GroupsCollection groups;

  uint32 GetNumGroups() const override;
  uint32 GetGroupEventCount(uint32 groupID) const override;
  uint32 GetGroupHash(uint32 groupID) const override;

  int Save(BinWritterRef wr) const;
};
