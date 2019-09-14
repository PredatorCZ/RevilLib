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

#include "LMTEvent.h"
#include "LMTFixupStorage.h"
#include "datas/reflectorRegistry.hpp"
#include <array>
#include <unordered_map>

typedef AnimEvents<PointerX86> EventTablePointerX86;
REFLECTOR_CREATE(EventTablePointerX86, 2, VARNAMES, TEMPLATE, eventRemaps)

typedef AnimEvents<PointerX64> EventTablePointerX64;
REFLECTOR_CREATE(EventTablePointerX64, 2, VARNAMES, TEMPLATE, eventRemaps)

template <class C, const int numGroups>
class AnimEvents_shared : public LMTAnimationEventV1_Internal {
public:
  typedef std::array<C, numGroups> GroupArray;
  typedef std::unique_ptr<GroupArray, std::deleter_hybrid> GroupsPtr;
  typedef std::vector<AnimEvent, std::allocator_hybrid<AnimEvent>>
      EventsCollection;

private:
  GroupsPtr groups;
  EventsCollection events[numGroups];
  std::vector<short> eventFrames[numGroups];

public:
  AnimEvents_shared() : groups(new typename GroupsPtr::element_type()) {}
  AnimEvents_shared(C *fromPtr, char *masterBuffer, bool swapEndian) {
    groups = GroupsPtr(reinterpret_cast<GroupArray *>(fromPtr), false);

    GroupArray &groupArray = *groups.get();
    int currentGroup = 0;

    for (auto &g : groupArray) {
      g.Fixup(masterBuffer, swapEndian);
      AnimEvent *groupEvents = g.events.GetData(masterBuffer);

      if (!groupEvents)
        continue;

      events[currentGroup] =
          EventsCollection(groupEvents, groupEvents + g.numEvents,
                           EventsCollection::allocator_type(groupEvents));

      int currentFrame = 0;

      eventFrames[currentGroup].reserve(events[currentGroup].size());

      for (auto &e : events[currentGroup]) {
        eventFrames[currentGroup].push_back(currentFrame);
        currentFrame += e.numFrames;
      }

      currentGroup++;
    }
  }

  void _ToXML(pugi::xml_node &node, int groupID) const override {
    ReflectorWrapConst<const C> reflEvent(&(*groups)[groupID]);
    reflEvent.ToXML(node, false);
  }

  void _FromXML(pugi::xml_node &node, int groupID) override {
    ReflectorWrap<C> reflEvent(&(*groups)[groupID]);
    reflEvent.FromXML(node, false);
  }

  int GetNumGroups() const override { return numGroups; }

  int GetGroupEventCount(int groupID) const override {
    return static_cast<int>(events[groupID].size());
  }

  const EventsCollection &GetEvents(int groupID) const override {
    return events[groupID];
  }

  EventsCollection &GetEvents(int groupID) override { return events[groupID]; }

  const short *GetRemaps(int groupID) const override {
    return (*groups)[groupID].eventRemaps;
  }

  int GetEventFrame(int groupID, int eventID) const override {
    return eventFrames[groupID][eventID];
  }

  void SetNumEvents(int groupID, int newSize) override {
    events[groupID].resize(newSize);
    (*groups)[groupID].numEvents = newSize;
  }

  void _Save(BinWritter *wr, LMTFixupStorage &storage) const override {
    const size_t cOff = wr->Tell();
    int curGroup = 0;

    for (auto &g : *groups) {
      wr->Write(g);
      storage.SaveFrom(cOff + offsetof(C, events) + sizeof(C) * curGroup++);
    }
  }

  bool _Is64bit() const override { return sizeof(C::events) == 8; }
};

void AnimEvent::SwapEndian() {
  FByteswapper(runEventBit);
  FByteswapper(numFrames);
}

template <template <class C> class PtrType>
void AnimEvents<PtrType>::SwapEndian() {
  for (auto &v : eventRemaps)
    FByteswapper(v);

  FByteswapper(numEvents);
}

template <template <class C> class PtrType>
void AnimEvents<PtrType>::Fixup(char *masterBuffer, bool swapEndian) {
  events.Fixup(masterBuffer, swapEndian);

  if (!swapEndian)
    return;

  SwapEndian();

  AnimEvent *_events = events.GetData(masterBuffer);

  for (int e = 0; e < numEvents; e++)
    _events[e].SwapEndian();
}

int LMTAnimationEventV1::GetVersion() const { return 1; }

LMTAnimationEventV1::EventCollection
LMTAnimationEventV1_Internal::GetEvents(int groupID, int eventID) const {
  const AnimEvent &cEvent = GetEvents(groupID)[eventID];
  EventCollection result;

  if (!cEvent.runEventBit)
    return result;

  for (int i = 0; i < 32; i++)
    if (cEvent.runEventBit & (1 << i))
      result.push_back(GetRemaps(groupID)[i]);

  return result;
}

/************************************************************************/
/***************************** EVENTS V2 ********************************/
/************************************************************************/

REFLECTOR_CREATE(AnimEventFrameV2, 1, VARNAMES, frame, type, dataType);

struct AnimEventV2 {
  typedef PointerX64<AnimEventFrameV2> FramesPtr;

  FramesPtr frames;
  uint64 numFrames;
  uint eventHash;
  EventFrameV2DataType dataType;

  void SwapEndian() {
    FByteswapper(numFrames);
    FByteswapper(eventHash);
    FByteswapper(dataType);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    frames.Fixup(masterBuffer, swapEndian);

    if (!swapEndian)
      return;

    SwapEndian();

    AnimEventFrameV2 *_frames = frames.GetData(masterBuffer);

    for (int e = 0; e < numFrames; e++)
      _frames[e].SwapEndian();
  }
};

struct AnimEventGroupV2 {
  typedef PointerX64<AnimEventV2> EventPtr;

  EventPtr events;
  uint64 numEvents;
  int groupHash;

  void SwapEndian() {
    FByteswapper(numEvents);
    FByteswapper(groupHash);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    events.Fixup(masterBuffer, swapEndian);

    if (swapEndian)
      SwapEndian();

    AnimEventV2 *_events = events.GetData(masterBuffer);

    for (int e = 0; e < numEvents; e++)
      _events[e].Fixup(masterBuffer, swapEndian);
  }
};

struct AnimEventsHeaderV2 {
  typedef PointerX64<AnimEventGroupV2> GroupPtr;

  GroupPtr eventGroups;
  uint64 numGroups;
  int totalNumEvents;
  int totalNumEventFrames;
  float numFrames;
  float loopFrame;
  int null00;
  uint collectionHash;

  void SwapEndian() {
    FByteswapper(numGroups);
    FByteswapper(collectionHash);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    eventGroups.Fixup(masterBuffer, swapEndian);

    if (swapEndian)
      SwapEndian();

    AnimEventGroupV2 *groups = eventGroups.GetData(masterBuffer);

    for (int e = 0; e < numGroups; e++)
      groups[e].Fixup(masterBuffer, swapEndian);
  }
};

class AnimEventV2_wrapper : public LMTAnimationEventV2Event {
public:
  typedef std::unique_ptr<AnimEventV2, std::deleter_hybrid> EventPtr;

private:
  EventPtr data;

public:
  AnimEventV2_wrapper() : data(new AnimEventV2) {}
  AnimEventV2_wrapper(AnimEventV2 *fromPtr, char *masterBuffer,
                      bool swapEndian) {
    data = EventPtr(fromPtr, false);
    data->Fixup(masterBuffer, swapEndian);

    AnimEventFrameV2 *rawFrames = data->frames.GetData(masterBuffer);

    frames = FramesCollection(rawFrames, rawFrames + data->numFrames,
                              FramesCollection::allocator_type(rawFrames));
  }

  uint GetHash() const override { return data->eventHash; }
  void SetHash(uint nHash) override { data->eventHash = nHash; }

  void _Save(BinWritter *wr, LMTFixupStorage &storage) const override {
    const size_t cOff = wr->Tell();

    wr->Write(*data);
    storage.SaveFrom(cOff + offsetof(AnimEventV2, frames));
  }
};

class AnimEventV2Group_wrapper : public LMTAnimationEventV2Group {
public:
  typedef std::unique_ptr<AnimEventGroupV2, std::deleter_hybrid> GroupPtr;

private:
  GroupPtr group;

public:
  AnimEventV2Group_wrapper() : group(new AnimEventGroupV2) {}
  AnimEventV2Group_wrapper(AnimEventGroupV2 *fromPtr, char *masterBuffer,
                           bool swapEndian) {
    group = GroupPtr(fromPtr, false);
    group->Fixup(masterBuffer, swapEndian);

    AnimEventV2 *rawEvents = group->events.GetData(masterBuffer);

    for (int e = 0; e < group->numEvents; e++)
      events.push_back(EventPtr(
          new AnimEventV2_wrapper(rawEvents + e, masterBuffer, swapEndian)));
  }

  uint GetHash() const override { return group->groupHash; }
  void SetHash(uint nHash) override { group->groupHash = nHash; }

  void _Save(BinWritter *wr, LMTFixupStorage &storage) const override {
    const size_t cOff = wr->Tell();

    wr->Write(*group);
    storage.SaveFrom(cOff + offsetof(AnimEventGroupV2, events));
  }
};

class AnimEventsV2_wrapper : public LMTAnimationEventV2_Internal {
public:
  typedef std::unique_ptr<AnimEventsHeaderV2, std::deleter_hybrid> HeaderPtr;

private:
  HeaderPtr header;

public:
  AnimEventsV2_wrapper() : header(new AnimEventsHeaderV2) {}
  AnimEventsV2_wrapper(AnimEventsHeaderV2 *fromPtr, char *masterBuffer,
                       bool swapEndian) {
    header = HeaderPtr(fromPtr, false);
    header->Fixup(masterBuffer, swapEndian);

    AnimEventGroupV2 *rawGroups = header->eventGroups.GetData(masterBuffer);

    for (int g = 0; g < header->numGroups; g++)
      groups.push_back(GroupPtr(new AnimEventV2Group_wrapper(
          rawGroups + g, masterBuffer, swapEndian)));
  }

  uint GetHash() const override { return header->collectionHash; }
  void SetHash(uint nHash) override { header->collectionHash = nHash; }

  void _Save(BinWritter *wr, LMTFixupStorage &storage) const override {
    const size_t cOff = wr->Tell();

    wr->Write(*header);
    storage.SaveFrom(cOff + offsetof(AnimEventsHeaderV2, eventGroups));
  }
};

int LMTAnimationEventV2::GetVersion() const { return 2; }

void AnimEventFrameV2::SwapEndian() { FByteswapper(frame); }

LMTAnimationEventV2Group *LMTAnimationEventV2Group::Create() {
  return new AnimEventV2Group_wrapper;
}

LMTAnimationEventV2Event *LMTAnimationEventV2Event::Create() {
  return new AnimEventV2_wrapper;
}

template <class C, int numG> static LMTAnimationEvent *_creattorBase() {
  return new AnimEvents_shared<C, numG>;
}

template <class C, int numG>
static LMTAnimationEvent *_creator(void *ptr, char *buff, bool endi) {
  return new AnimEvents_shared<C, numG>(static_cast<C *>(ptr), buff, endi);
}

static LMTAnimationEvent *_creattorBase2() { return new AnimEventsV2_wrapper; }

static LMTAnimationEvent *_creator2(void *ptr, char *buff, bool endi) {
  return new AnimEventsV2_wrapper(static_cast<AnimEventsHeaderV2 *>(ptr), buff,
                                  endi);
}

static const std::unordered_map<short, LMTAnimationEvent *(*)()> eventRegistry =
    {{0x108, _creattorBase<EventTablePointerX64, 2>},
     {0x104, _creattorBase<EventTablePointerX86, 2>},
     {0x208, _creattorBase<EventTablePointerX64, 4>},
     {0x204, _creattorBase<EventTablePointerX86, 4>},
     {0x308, _creattorBase2}};

static const std::unordered_map<short,
                                LMTAnimationEvent *(*)(void *, char *, bool)>
    eventRegistryLink = {{0x108, _creator<EventTablePointerX64, 2>},
                         {0x104, _creator<EventTablePointerX86, 2>},
                         {0x208, _creator<EventTablePointerX64, 4>},
                         {0x204, _creator<EventTablePointerX86, 4>},
                         {0x308, _creator2}};

REGISTER_ENUMS(EventFrameV2DataType, EventFrameV2Type);

LMTAnimationEvent *
LMTAnimationEvent::Create(const LMTConstructorProperties &props) {
  short item = reinterpret_cast<const short &>(props);
  RegisterLocalEnums();

  if (!eventRegistry.count(item))
    return nullptr;

  if (props.dataStart)
    return eventRegistryLink.at(item)(props.dataStart, props.masterBuffer,
                                      props.swappedEndian);
  else
    return eventRegistry.at(item)();
}

int LMTAnimationEventV2_Internal::GetNumGroups() const {
  return static_cast<int>(groups.size());
}

uint LMTAnimationEventV2_Internal::GetGroupHash(int groupID) const {
  return groups[groupID]->GetHash();
}

int LMTAnimationEventV2_Internal::GetGroupEventCount(int groupID) const {
  return static_cast<int>(groups[groupID]->events.size());
}