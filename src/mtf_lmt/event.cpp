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

#include "event.hpp"
#include "datas/reflector_xml.hpp"
#include "fixup_storage.hpp"

#include <array>
#include <unordered_map>

REFLECTOR_CREATE((AnimEvents<esPointerX86>), 2, VARNAMES, TEMPLATE, eventRemaps)
REFLECTOR_CREATE((AnimEvents<esPointerX64>), 2, VARNAMES, TEMPLATE, eventRemaps)

template <class C, const uint32 numGroups>
class AnimEvents_shared : public LMTAnimationEventV1_Internal {
public:
  typedef std::array<C, numGroups> GroupArray;
  typedef std::unique_ptr<GroupArray, es::deleter_hybrid> GroupsPtr;
  typedef std::vector<AnimEvent, es::allocator_hybrid<AnimEvent>>
      EventsCollection;

private:
  GroupsPtr groups;
  EventsCollection events[numGroups];
  std::vector<int16> eventFrames[numGroups];

public:
  AnimEvents_shared() : groups(new typename GroupsPtr::element_type()) {}
  AnimEvents_shared(C *fromPtr, char *masterBuffer, bool swapEndian) {
    groups = GroupsPtr(reinterpret_cast<GroupArray *>(fromPtr), false);

    GroupArray &groupArray = *groups.get();
    uint32 currentGroup = 0;

    for (auto &g : groupArray) {
      g.Fixup(masterBuffer, swapEndian);
      AnimEvent *groupEvents = g.events;

      if (!groupEvents)
        continue;

      events[currentGroup] =
          EventsCollection(groupEvents, groupEvents + g.numEvents,
                           EventsCollection::allocator_type(groupEvents));

      uint32 currentFrame = 0;

      eventFrames[currentGroup].reserve(events[currentGroup].size());

      for (auto &e : events[currentGroup]) {
        eventFrames[currentGroup].push_back(currentFrame);
        currentFrame += e.numFrames;
      }

      currentGroup++;
    }
  }

  void _ToXML(pugi::xml_node &node, uint32 groupID) const override {
    ReflectorWrapConst<C> reflEvent((*groups)[groupID]);
    ReflectorXMLUtil::Save(reflEvent, node);
  }

  void _FromXML(pugi::xml_node &node, uint32 groupID) override {
    ReflectorWrap<C> reflEvent((*groups)[groupID]);
    ReflectorXMLUtil::Load(reflEvent, node);
  }

  uint32 GetNumGroups() const override { return numGroups; }

  uint32 GetGroupEventCount(uint32 groupID) const override {
    return static_cast<uint32>(events[groupID].size());
  }

  const EventsCollection &GetEvents(uint32 groupID) const override {
    return events[groupID];
  }

  EventsCollection &GetEvents(uint32 groupID) override {
    return events[groupID];
  }

  const uint16 *GetRemaps(uint32 groupID) const override {
    return (*groups)[groupID].eventRemaps;
  }

  int32 GetEventFrame(uint32 groupID, uint32 eventID) const override {
    return eventFrames[groupID][eventID];
  }

  void SetNumEvents(uint32 groupID, uint32 newSize) override {
    events[groupID].resize(newSize);
    (*groups)[groupID].numEvents = newSize;
  }

  void _Save(BinWritterRef wr, LMTFixupStorage &storage) const override {
    const size_t cOff = wr.Tell();
    uint32 curGroup = 0;

    for (auto &g : *groups) {
      wr.Write(g);
      storage.SaveFrom(cOff + offsetof(C, events) + sizeof(C) * curGroup++);
    }
  }

  bool _Is64bit() const override {
    return sizeof(std::declval<C>().events) == 8;
  }
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
  FByteswapper(events);
}

template <template <class C> class PtrType>
void AnimEvents<PtrType>::Fixup(char *masterBuffer, bool swapEndian) {
  if (events.Fixed())
    return;

  if (swapEndian)
    SwapEndian();

  events.Fixup(masterBuffer, true);

  if (!swapEndian)
    return;

  AnimEvent *_events = events;

  for (uint32 e = 0; e < numEvents; e++)
    _events[e].SwapEndian();
}

uint32 LMTAnimationEventV1::GetVersion() const { return 1; }

LMTAnimationEventV1::EventCollection
LMTAnimationEventV1_Internal::GetEvents(uint32 groupID, uint32 eventID) const {
  const AnimEvent &cEvent = GetEvents(groupID)[eventID];
  EventCollection result;

  if (!cEvent.runEventBit)
    return result;

  for (uint32 i = 0; i < 32; i++)
    if (cEvent.runEventBit & (1 << i))
      result.push_back(GetRemaps(groupID)[i]);

  return result;
}

/************************************************************************/
/***************************** EVENTS V2 ********************************/
/************************************************************************/

REFLECTOR_CREATE(AnimEventFrameV2, 1, VARNAMES, frame, type, dataType);

struct AnimEventV2 {
  typedef esPointerX64<AnimEventFrameV2> FramesPtr;

  FramesPtr frames;
  uint64 numFrames;
  uint32 eventHash;
  EventFrameV2DataType dataType;

  void SwapEndian() {
    FByteswapper(frames);
    FByteswapper(numFrames);
    FByteswapper(eventHash);
    FByteswapper(dataType);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    if (frames.Fixed())
      return;

    if (swapEndian)
      SwapEndian();

    frames.Fixup(masterBuffer, true);

    if (!swapEndian)
      return;

    AnimEventFrameV2 *_frames = frames;

    for (uint32 e = 0; e < numFrames; e++)
      _frames[e].SwapEndian();
  }
};

struct AnimEventGroupV2 {
  typedef esPointerX64<AnimEventV2> EventPtr;

  EventPtr events;
  uint64 numEvents;
  uint32 groupHash;

  void SwapEndian() {
    FByteswapper(events);
    FByteswapper(numEvents);
    FByteswapper(groupHash);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    if (events.Fixed())
      return;

    if (swapEndian)
      SwapEndian();

    events.Fixup(masterBuffer, true);

    AnimEventV2 *_events = events;

    for (uint32 e = 0; e < numEvents; e++)
      _events[e].Fixup(masterBuffer, swapEndian);
  }
};

struct AnimEventsHeaderV2 {
  typedef esPointerX64<AnimEventGroupV2> GroupPtr;

  GroupPtr eventGroups;
  uint64 numGroups;
  uint32 totalNumEvents;
  uint32 totalNumEventFrames;
  float numFrames;
  float loopFrame;
  uint32 null00;
  uint32 collectionHash;

  void SwapEndian() {
    FByteswapper(eventGroups);
    FByteswapper(numGroups);
    FByteswapper(collectionHash);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    if (eventGroups.Fixed())
      return;

    if (swapEndian)
      SwapEndian();

    eventGroups.Fixup(masterBuffer, true);

    AnimEventGroupV2 *groups = eventGroups;

    for (uint32 e = 0; e < numGroups; e++)
      groups[e].Fixup(masterBuffer, swapEndian);
  }
};

class AnimEventV2_wrapper : public LMTAnimationEventV2Event {
public:
  typedef std::unique_ptr<AnimEventV2, es::deleter_hybrid> EventPtr;

private:
  EventPtr data;

public:
  AnimEventV2_wrapper() : data(new AnimEventV2) {}
  AnimEventV2_wrapper(AnimEventV2 *fromPtr, char *masterBuffer,
                      bool swapEndian) {
    data = EventPtr(fromPtr, false);
    data->Fixup(masterBuffer, swapEndian);

    AnimEventFrameV2 *rawFrames = data->frames;

    frames = FramesCollection(rawFrames, rawFrames + data->numFrames,
                              FramesCollection::allocator_type(rawFrames));
  }

  uint32 GetHash() const override { return data->eventHash; }
  void SetHash(uint32 nHash) override { data->eventHash = nHash; }

  void _Save(BinWritterRef wr, LMTFixupStorage &storage) const override {
    const size_t cOff = wr.Tell();

    wr.Write(*data);
    storage.SaveFrom(cOff + offsetof(AnimEventV2, frames));
  }
};

class AnimEventV2Group_wrapper : public LMTAnimationEventV2Group {
public:
  typedef std::unique_ptr<AnimEventGroupV2, es::deleter_hybrid> GroupPtr;

private:
  GroupPtr group;

public:
  AnimEventV2Group_wrapper() : group(new AnimEventGroupV2) {}
  AnimEventV2Group_wrapper(AnimEventGroupV2 *fromPtr, char *masterBuffer,
                           bool swapEndian) {
    group = GroupPtr(fromPtr, false);
    group->Fixup(masterBuffer, swapEndian);

    AnimEventV2 *rawEvents = group->events;

    for (uint32 e = 0; e < group->numEvents; e++)
      events.push_back(EventPtr(
          new AnimEventV2_wrapper(rawEvents + e, masterBuffer, swapEndian)));
  }

  uint32 GetHash() const override { return group->groupHash; }
  void SetHash(uint32 nHash) override { group->groupHash = nHash; }

  void _Save(BinWritterRef wr, LMTFixupStorage &storage) const override {
    const size_t cOff = wr.Tell();

    wr.Write(*group);
    storage.SaveFrom(cOff + offsetof(AnimEventGroupV2, events));
  }
};

class AnimEventsV2_wrapper : public LMTAnimationEventV2_Internal {
public:
  typedef std::unique_ptr<AnimEventsHeaderV2, es::deleter_hybrid> HeaderPtr;

private:
  HeaderPtr header;

public:
  AnimEventsV2_wrapper() : header(new AnimEventsHeaderV2) {}
  AnimEventsV2_wrapper(AnimEventsHeaderV2 *fromPtr, char *masterBuffer,
                       bool swapEndian) {
    header = HeaderPtr(fromPtr, false);
    header->Fixup(masterBuffer, swapEndian);

    AnimEventGroupV2 *rawGroups = header->eventGroups;

    for (uint32 g = 0; g < header->numGroups; g++)
      groups.push_back(GroupPtr(new AnimEventV2Group_wrapper(
          rawGroups + g, masterBuffer, swapEndian)));
  }

  uint32 GetHash() const override { return header->collectionHash; }
  void SetHash(uint32 nHash) override { header->collectionHash = nHash; }

  void _Save(BinWritterRef wr, LMTFixupStorage &storage) const override {
    const size_t cOff = wr.Tell();

    wr.Write(*header);
    storage.SaveFrom(cOff + offsetof(AnimEventsHeaderV2, eventGroups));
  }
};

uint32 LMTAnimationEventV2::GetVersion() const { return 2; }

void AnimEventFrameV2::SwapEndian() { FByteswapper(frame); }

LMTAnimationEventV2Group *LMTAnimationEventV2Group::Create() {
  return new AnimEventV2Group_wrapper;
}

LMTAnimationEventV2Event *LMTAnimationEventV2Event::Create() {
  return new AnimEventV2_wrapper;
}

template <class C, uint32 numG> static LMTAnimationEvent *_creattorBase() {
  return new AnimEvents_shared<C, numG>;
}

template <class C, uint32 numG>
static LMTAnimationEvent *_creator(void *ptr, char *buff, bool endi) {
  return new AnimEvents_shared<C, numG>(static_cast<C *>(ptr), buff, endi);
}

static LMTAnimationEvent *_creattorBase2() { return new AnimEventsV2_wrapper; }

static LMTAnimationEvent *_creator2(void *ptr, char *buff, bool endi) {
  return new AnimEventsV2_wrapper(static_cast<AnimEventsHeaderV2 *>(ptr), buff,
                                  endi);
}

static const std::unordered_map<uint16, LMTAnimationEvent *(*)()> eventRegistry{
    {0x108, _creattorBase<AnimEvents<esPointerX64>, 2>},
    {0x104, _creattorBase<AnimEvents<esPointerX86>, 2>},
    {0x208, _creattorBase<AnimEvents<esPointerX64>, 4>},
    {0x204, _creattorBase<AnimEvents<esPointerX86>, 4>},
    {0x308, _creattorBase2},
};

static const std::unordered_map<uint16,
                                LMTAnimationEvent *(*)(void *, char *, bool)>
    eventRegistryLink{
        {0x108, _creator<AnimEvents<esPointerX64>, 2>},
        {0x104, _creator<AnimEvents<esPointerX86>, 2>},
        {0x208, _creator<AnimEvents<esPointerX64>, 4>},
        {0x204, _creator<AnimEvents<esPointerX86>, 4>},
        {0x308, _creator2},
    };

LMTAnimationEvent *
LMTAnimationEvent::Create(const LMTConstructorProperties &props) {
  uint16 item = reinterpret_cast<const uint16 &>(props);

  REFLECTOR_REGISTER(EventFrameV2DataType, EventFrameV2Type);

  if (!eventRegistry.count(item))
    return nullptr;

  if (props.dataStart)
    return eventRegistryLink.at(item)(props.dataStart, props.masterBuffer,
                                      props.swappedEndian);
  else
    return eventRegistry.at(item)();
}

uint32 LMTAnimationEventV2_Internal::GetNumGroups() const {
  return static_cast<uint32>(groups.size());
}

uint32 LMTAnimationEventV2_Internal::GetGroupHash(uint32 groupID) const {
  return groups[groupID]->GetHash();
}

uint32 LMTAnimationEventV2_Internal::GetGroupEventCount(uint32 groupID) const {
  return static_cast<uint32>(groups[groupID]->events.size());
}
