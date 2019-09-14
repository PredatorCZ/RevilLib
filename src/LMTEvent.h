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

struct AnimEvent {
  int runEventBit;
  int numFrames;

  Reflector::xmlNodePtr ToXML(pugi::xml_node &node) const;
  int FromXML(pugi::xml_node &node);
  void SwapEndian();
};

template <template <class C> class PtrType> struct AnimEvents {
  DECLARE_REFLECTOR;

  short eventRemaps[32];
  int numEvents;
  PtrType<AnimEvent> events;

  void SwapEndian();
  void Fixup(char *masterBuffer, bool swapEndian);
};

class LMTAnimationEventV1_Internal : public LMTAnimationEventV1 {
  virtual void _ToXML(pugi::xml_node &node, int groupID) const = 0;
  virtual void _FromXML(pugi::xml_node &node, int groupID) = 0;
  virtual void _Save(BinWritter *wr, LMTFixupStorage &fixups) const = 0;
  virtual bool _Is64bit() const = 0;

public:
  typedef std::vector<AnimEvent, std::allocator_hybrid<AnimEvent>>
      EventsCollection;

  virtual const EventsCollection &GetEvents(int groupID) const = 0;
  virtual EventsCollection &GetEvents(int groupID) = 0;
  virtual const short *GetRemaps(int groupID) const = 0;
  virtual void SetNumEvents(int groupID, int newSize) = 0;

  int ToXML(pugi::xml_node &node, bool standAlone) const override;
  int FromXML(pugi::xml_node &node) override;
  int Save(BinWritter *wr) const;
  int SaveBuffer(BinWritter *wr, LMTFixupStorage &fixups) const;
  EventCollection GetEvents(int groupID, int eventID) const override;
};

REFLECTOR_CREATE(EventFrameV2DataType, ENUM, 2, CLASS, 16, Int8, Int32, Float,
                 Bool = 4);
REFLECTOR_CREATE(EventFrameV2Type, ENUM, 2, CLASS, 16, Scalar, Scalar2, Scalar3,
                 PackedFloat, PackedBitFlags = 5, PackedInt);

struct AnimEventFrameV2 {
  DECLARE_REFLECTOR;

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
  typedef std::vector<AnimEventFrameV2, std::allocator_hybrid<AnimEventFrameV2>>
      FramesCollection;

  FramesCollection frames;

  virtual uint GetHash() const = 0;
  virtual void SetHash(uint nHash) = 0;
  virtual void _Save(BinWritter *wr, LMTFixupStorage &storage) const = 0;

  static LMTAnimationEventV2Event *Create();
  static LMTAnimationEventV2Event *FromXML(pugi::xml_node &node);

  virtual ~LMTAnimationEventV2Event() {}
};

class LMTAnimationEventV2Group {
public:
  typedef std::unique_ptr<LMTAnimationEventV2Event> EventPtr;
  typedef std::vector<EventPtr> EventsCollection;

  EventsCollection events;

  virtual uint GetHash() const = 0;
  virtual void SetHash(uint nHash) = 0;
  virtual void _Save(BinWritter *wr, LMTFixupStorage &storage) const = 0;

  static LMTAnimationEventV2Group *Create();
  static LMTAnimationEventV2Group *FromXML(pugi::xml_node &node);

  virtual ~LMTAnimationEventV2Group() {}
};

class LMTAnimationEventV2_Internal : public LMTAnimationEventV2 {
  int ToXML(pugi::xml_node &node, bool standAlone) const;
  int FromXML(pugi::xml_node &node);
  virtual void _Save(BinWritter *wr, LMTFixupStorage &storage) const = 0;

public:
  typedef std::unique_ptr<LMTAnimationEventV2Group> GroupPtr;
  typedef std::vector<GroupPtr> GroupsCollection;

  GroupsCollection groups;

  int GetNumGroups() const override;
  int GetGroupEventCount(int groupID) const override;
  uint GetGroupHash(int groupID) const override;

  int Save(BinWritter *wr) const;
};
