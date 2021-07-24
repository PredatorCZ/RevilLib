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
#include "pugixml.hpp"

#include <array>
#include <map>

REFLECTOR_CREATE((AnimEvents<esPointerX86>), 2, VARNAMES, TEMPLATE, eventRemaps)
REFLECTOR_CREATE((AnimEvents<esPointerX64>), 2, VARNAMES, TEMPLATE, eventRemaps)

template struct AnimEvents<esPointerX64>;
template struct AnimEvents<esPointerX86>;

template <class C, size_t numGroups>
class AnimEvents_shared : public LMTAnimationEventV1_Internal {
public:
  using GroupArray = std::array<C, numGroups>;
  using GroupsPtr = uni::Element<GroupArray>;
  using EventsCollection =
      std::vector<AnimEvent, es::allocator_hybrid<AnimEvent>>;

private:
  GroupsPtr groups;
  EventsCollection events[numGroups];
  std::vector<int16> eventFrames[numGroups];

public:
  AnimEvents_shared() : groups(new GroupArray()) {}
  AnimEvents_shared(C *fromPtr, char *masterBuffer, bool swapEndian) {
    groups = GroupsPtr(reinterpret_cast<GroupArray *>(fromPtr), false);

    GroupArray &groupArray = *groups.get();
    size_t currentGroup = 0;

    for (auto &g : groupArray) {
      g.Fixup(masterBuffer, swapEndian);
      AnimEvent *groupEvents = g.events;

      if (!groupEvents) {
        continue;
      }

      es::allocator_hybrid_base::LinkStorage(events[currentGroup], groupEvents,
                                             g.numEvents);

      size_t currentFrame = 0;

      eventFrames[currentGroup].reserve(events[currentGroup].size());

      for (auto &e : events[currentGroup]) {
        eventFrames[currentGroup].push_back(currentFrame);
        currentFrame += e.numFrames;
      }

      currentGroup++;
    }
  }

  void ReflectToXML(pugi::xml_node node, size_t groupID) const override {
    ReflectorWrap<const C> reflEvent((*groups)[groupID]);
    ReflectorXMLUtil::Save(reflEvent, node);
  }

  void ReflectFromXML(pugi::xml_node node, size_t groupID) override {
    ReflectorWrap<C> reflEvent((*groups)[groupID]);
    ReflectorXMLUtil::Load(reflEvent, node);
  }

  size_t GetNumGroups() const override { return numGroups; }

  size_t GetGroupEventCount(size_t groupID) const override {
    return events[groupID].size();
  }

  const EventsCollection &GetEvents(size_t groupID) const override {
    return events[groupID];
  }

  EventsCollection &GetEvents(size_t groupID) override {
    return events[groupID];
  }

  const uint16 *GetRemaps(size_t groupID) const override {
    return (*groups)[groupID].eventRemaps;
  }

  int32 GetEventFrame(size_t groupID, size_t eventID) const override {
    return eventFrames[groupID][eventID];
  }

  void SetNumEvents(size_t groupID, size_t newSize) override {
    events[groupID].resize(newSize);
    (*groups)[groupID].numEvents = newSize;
  }

  void SaveInternal(BinWritterRef wr, LMTFixupStorage &storage) const override {
    const size_t cOff = wr.Tell();
    size_t curGroup = 0;

    for (auto &g : *groups) {
      wr.Write(g);
      storage.SaveFrom(cOff + offsetof(C, events) + sizeof(C) * curGroup++);
    }
  }

  bool Is64bit() const override {
    return sizeof(std::declval<C>().events) == 8;
  }
};

void AnimEvent::SwapEndian() {
  FByteswapper(runEventBit);
  FByteswapper(numFrames);
}

template <template <class C> class PtrType>
void AnimEvents<PtrType>::SwapEndian() {
  for (auto &v : eventRemaps) {
    FByteswapper(v);
  }

  FByteswapper(numEvents);
  FByteswapper(events);
}

template <template <class C> class PtrType>
void AnimEvents<PtrType>::Fixup(char *masterBuffer, bool swapEndian) {
  if (events.Fixed()) {
    return;
  }

  if (swapEndian) {
    SwapEndian();
  }

  events.Fixup(masterBuffer);

  if (!swapEndian) {
    return;
  }

  AnimEvent *events_ = events;

  for (size_t e = 0; e < numEvents; e++) {
    events_[e].SwapEndian();
  }
}

size_t LMTAnimationEventV1_Internal::GetVersion() const { return 1; }

LMTAnimationEventV1::EventCollection
LMTAnimationEventV1_Internal::GetEvents(size_t groupID, size_t eventID) const {
  const AnimEvent &cEvent = GetEvents(groupID)[eventID];
  EventCollection result;

  if (!cEvent.runEventBit) {
    return result;
  }

  using TriggerType = decltype(AnimEvent::runEventBit);
  const size_t numEvents = sizeof(TriggerType) * 8;

  for (size_t i = 0; i < numEvents; i++) {
    if (cEvent.runEventBit & (TriggerType(1) << i)) {
      result.push_back(GetRemaps(groupID)[i]);
    }
  }

  return result;
}

/************************************************************************/
/***************************** EVENTS V2 ********************************/
/************************************************************************/

REFLECTOR_CREATE(AnimEventFrameV2, 1, VARNAMES, frame, type, dataType);

struct AnimEventV2 {
  using FramesPtr = esPointerX64<AnimEventFrameV2>;

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
    if (frames.Fixed()) {
      return;
    }

    if (swapEndian) {
      SwapEndian();
    }

    frames.Fixup(masterBuffer);

    if (!swapEndian) {
      return;
    }

    AnimEventFrameV2 *frames_ = frames;

    for (size_t e = 0; e < numFrames; e++) {
      frames_[e].SwapEndian();
    }
  }
};

struct AnimEventGroupV2 {
  using EventPtr = esPointerX64<AnimEventV2>;

  EventPtr events;
  uint64 numEvents;
  uint32 groupHash;

  void SwapEndian() {
    FByteswapper(events);
    FByteswapper(numEvents);
    FByteswapper(groupHash);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    if (events.Fixed()) {
      return;
    }

    if (swapEndian) {
      SwapEndian();
    }

    events.Fixup(masterBuffer);

    AnimEventV2 *events_ = events;

    for (size_t e = 0; e < numEvents; e++) {
      events_[e].Fixup(masterBuffer, swapEndian);
    }
  }
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

  void SwapEndian() {
    FByteswapper(eventGroups);
    FByteswapper(numGroups);
    FByteswapper(collectionHash);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    if (eventGroups.Fixed()) {
      return;
    }

    if (swapEndian) {
      SwapEndian();
    }

    eventGroups.Fixup(masterBuffer);

    AnimEventGroupV2 *groups = eventGroups;

    for (size_t e = 0; e < numGroups; e++) {
      groups[e].Fixup(masterBuffer, swapEndian);
    }
  }
};

class AnimEventV2_wrapper : public LMTAnimationEventV2Event {
public:
  using EventPtr = uni::Element<AnimEventV2>;

private:
  EventPtr data;

public:
  AnimEventV2_wrapper() : data(new AnimEventV2) {}
  AnimEventV2_wrapper(AnimEventV2 *fromPtr, char *masterBuffer,
                      bool swapEndian) {
    data = EventPtr(fromPtr, false);
    data->Fixup(masterBuffer, swapEndian);

    AnimEventFrameV2 *rawFrames = data->frames;
    es::allocator_hybrid_base::LinkStorage(frames, rawFrames, data->numFrames);
  }

  uint32 GetHash() const override { return data->eventHash; }
  void SetHash(uint32 nHash) override { data->eventHash = nHash; }

  void SaveInternal(BinWritterRef wr, LMTFixupStorage &storage) const override {
    const size_t cOff = wr.Tell();

    wr.Write(*data);
    storage.SaveFrom(cOff + offsetof(AnimEventV2, frames));
  }
};

class AnimEventV2Group_wrapper : public LMTAnimationEventV2Group {
public:
  using GroupPtr = uni::Element<AnimEventGroupV2>;

private:
  GroupPtr group;

public:
  AnimEventV2Group_wrapper() : group(new AnimEventGroupV2) {}
  AnimEventV2Group_wrapper(AnimEventGroupV2 *fromPtr, char *masterBuffer,
                           bool swapEndian) {
    group = GroupPtr(fromPtr, false);
    group->Fixup(masterBuffer, swapEndian);

    AnimEventV2 *rawEvents = group->events;

    for (size_t e = 0; e < group->numEvents; e++) {
      events.emplace_back(std::make_unique<AnimEventV2_wrapper>(
          rawEvents + e, masterBuffer, swapEndian));
    }
  }

  uint32 GetHash() const override { return group->groupHash; }
  void SetHash(uint32 nHash) override { group->groupHash = nHash; }

  void SaveInternal(BinWritterRef wr, LMTFixupStorage &storage) const override {
    const size_t cOff = wr.Tell();

    wr.Write(*group);
    storage.SaveFrom(cOff + offsetof(AnimEventGroupV2, events));
  }
};

class AnimEventsV2_wrapper : public LMTAnimationEventV2_Internal {
public:
  using HeaderPtr = uni::Element<AnimEventsHeaderV2>;

private:
  HeaderPtr header;

public:
  AnimEventsV2_wrapper() : header(new AnimEventsHeaderV2) {}
  AnimEventsV2_wrapper(AnimEventsHeaderV2 *fromPtr, char *masterBuffer,
                       bool swapEndian) {
    header = HeaderPtr(fromPtr, false);
    header->Fixup(masterBuffer, swapEndian);

    AnimEventGroupV2 *rawGroups = header->eventGroups;

    for (size_t g = 0; g < header->numGroups; g++) {
      groups.emplace_back(std::make_unique<AnimEventV2Group_wrapper>(
          rawGroups + g, masterBuffer, swapEndian));
    }
  }

  uint32 GetHash() const override { return header->collectionHash; }
  void SetHash(uint32 nHash) override { header->collectionHash = nHash; }

  void SaveInternal(BinWritterRef wr, LMTFixupStorage &storage) const override {
    const size_t cOff = wr.Tell();

    wr.Write(*header);
    storage.SaveFrom(cOff + offsetof(AnimEventsHeaderV2, eventGroups));
  }
};

void AnimEventFrameV2::SwapEndian() { FByteswapper(frame); }

LMTAnimationEventV2Group::Ptr LMTAnimationEventV2Group::Create() {
  return std::make_unique<AnimEventV2Group_wrapper>();
}

LMTAnimationEventV2Event::Ptr LMTAnimationEventV2Event::Create() {
  return std::make_unique<AnimEventV2_wrapper>();
}

using ptr_type_ = std::unique_ptr<LMTAnimationEvent>;

template <class C, uint32 numG> static ptr_type_ creatorBase_() {
  return std::make_unique<AnimEvents_shared<C, numG>>();
}

template <class C, uint32 numG>
static ptr_type_ creator_(void *ptr, char *buff, bool endi) {
  return std::make_unique<AnimEvents_shared<C, numG>>(static_cast<C *>(ptr),
                                                      buff, endi);
}

static ptr_type_ creatorBase2_() {
  return std::make_unique<AnimEventsV2_wrapper>();
}

static ptr_type_ creator2_(void *ptr, char *buff, bool endi) {
  return std::make_unique<AnimEventsV2_wrapper>(
      static_cast<AnimEventsHeaderV2 *>(ptr), buff, endi);
}

static const std::map<LMTConstructorPropertiesBase, decltype(&creatorBase2_)>
    eventRegistry{
        // clang-format off
        {{LMTArchType::X64, LMTVersion::V_22}, creatorBase_<AnimEvents<esPointerX64>, 2>},
        {{LMTArchType::X86, LMTVersion::V_22}, creatorBase_<AnimEvents<esPointerX86>, 2>},
        {{LMTArchType::X64, LMTVersion::V_56}, creatorBase_<AnimEvents<esPointerX64>, 4>},
        {{LMTArchType::X86, LMTVersion::V_56}, creatorBase_<AnimEvents<esPointerX86>, 4>},
        {{LMTArchType::X64, LMTVersion::V_66}, creatorBase2_},
        // clang-format on
    };

static const std::map<LMTConstructorPropertiesBase, decltype(&creator2_)>
    eventRegistryLink{
        // clang-format off
        {{LMTArchType::X64, LMTVersion::V_22}, creator_<AnimEvents<esPointerX64>, 2>},
        {{LMTArchType::X86, LMTVersion::V_22}, creator_<AnimEvents<esPointerX86>, 2>},
        {{LMTArchType::X64, LMTVersion::V_56}, creator_<AnimEvents<esPointerX64>, 4>},
        {{LMTArchType::X86, LMTVersion::V_56}, creator_<AnimEvents<esPointerX86>, 4>},
        {{LMTArchType::X64, LMTVersion::V_66}, creator2_},
        // clang-format on
    };

ptr_type_ LMTAnimationEvent::Create(const LMTConstructorProperties &props) {
  RegisterReflectedTypes<EventFrameV2DataType, EventFrameV2Type>();

  if (props.dataStart) {
    return eventRegistryLink.at(props)(props.dataStart, props.masterBuffer,
                                       props.swappedEndian);
  } else {
    return eventRegistry.at(props)();
  }
}

size_t LMTAnimationEventV2_Internal::GetVersion() const { return 2; }

size_t LMTAnimationEventV2_Internal::GetNumGroups() const {
  return groups.size();
}

uint32 LMTAnimationEventV2_Internal::GetGroupHash(size_t groupID) const {
  return groups[groupID]->GetHash();
}

size_t LMTAnimationEventV2_Internal::GetGroupEventCount(size_t groupID) const {
  return groups[groupID]->events.size();
}
