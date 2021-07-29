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
#include "datas/reflector.hpp"
#include "internal.hpp"

class AnimEvent {
public:
  uint32 runEventBit;
  uint32 numFrames;

  void Save(pugi::xml_node node) const;
  void Load(pugi::xml_node node);
  void SwapEndian();
};

template <template <class C> class PtrType> struct AnimEvents {
  uint16 eventRemaps[32];
  uint32 numEvents;
  PtrType<AnimEvent> events;

  void SwapEndian();
  void Fixup(char *masterBuffer, bool swapEndian);
};

class LMTAnimationEventV1_Internal : public LMTAnimationEventV1 {
  virtual void ReflectToXML(pugi::xml_node node, size_t groupID) const = 0;
  virtual void ReflectFromXML(pugi::xml_node node, size_t groupID) = 0;
  virtual void SaveInternal(BinWritterRef wr,
                            LMTFixupStorage &fixups) const = 0;
  virtual bool Is64bit() const = 0;

public:
  using EventsCollection =
      std::vector<AnimEvent, es::allocator_hybrid<AnimEvent>>;

  virtual const EventsCollection &GetEvents(size_t groupID) const = 0;
  virtual EventsCollection &GetEvents(size_t groupID) = 0;
  virtual const uint16 *GetRemaps(size_t groupID) const = 0;
  virtual void SetNumEvents(size_t groupID, size_t newSize) = 0;

  size_t GetVersion() const override;
  void Save(pugi::xml_node node, bool standAlone) const override;
  void Load(pugi::xml_node node) override;
  void Save(BinWritterRef wr) const;
  void SaveBuffer(BinWritterRef wr, LMTFixupStorage &fixups) const;
  EventCollection GetEvents(size_t groupID, size_t eventID) const override;
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
  void SwapEndian();
};

class LMTAnimationEventV2Event {
public:
  using FramesCollection =
      std::vector<AnimEventFrameV2, es::allocator_hybrid<AnimEventFrameV2>>;
  using Ptr = std::unique_ptr<LMTAnimationEventV2Event>;

  FramesCollection frames;

  virtual uint32 GetHash() const = 0;
  virtual void SetHash(uint32 nHash) = 0;
  virtual void SaveInternal(BinWritterRef wr,
                            LMTFixupStorage &storage) const = 0;
  virtual ~LMTAnimationEventV2Event() {}

  void Load(pugi::xml_node node);

  static Ptr Create();
};

class LMTAnimationEventV2Group {
public:
  using EventsCollection = std::vector<LMTAnimationEventV2Event::Ptr>;
  using Ptr = std::unique_ptr<LMTAnimationEventV2Group>;

  EventsCollection events;

  virtual uint32 GetHash() const = 0;
  virtual void SetHash(uint32 nHash) = 0;
  virtual void SaveInternal(BinWritterRef wr,
                            LMTFixupStorage &storage) const = 0;
  virtual ~LMTAnimationEventV2Group() {}

  void Load(pugi::xml_node node);

  static Ptr Create();
};

class LMTAnimationEventV2_Internal : public LMTAnimationEventV2 {
  void Save(pugi::xml_node node, bool standAlone) const override;
  void Load(pugi::xml_node node) override;
  virtual void SaveInternal(BinWritterRef wr,
                            LMTFixupStorage &storage) const = 0;

public:
  using GroupsCollection = std::vector<LMTAnimationEventV2Group::Ptr>;

  GroupsCollection groups;

  size_t GetVersion() const override;
  size_t GetNumGroups() const override;
  size_t GetGroupEventCount(size_t groupID) const override;
  uint32 GetGroupHash(size_t groupID) const override;

  void Save(BinWritterRef wr) const;
};
