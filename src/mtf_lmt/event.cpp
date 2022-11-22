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
#include "datas/reflector.hpp"

#include "event.inl"
#include <optional>

REFLECT(CLASS(AnimEventFrameV2), MEMBER(frame), MEMBER(type), MEMBER(dataType));

REFLECT(CLASS(AnimEventV2), MEMBER(eventHash), MEMBER(dataType));

REFLECT(CLASS(AnimEventGroupV2), MEMBER(groupHash));

REFLECT(CLASS(AnimEventsHeaderV2), MEMBER(totalNumEvents),
        MEMBER(totalNumEventFrames), MEMBER(numFrames), MEMBER(loopFrame),
        MEMBER(null00), MEMBER(collectionHash));

void FByteswapper(AnimEventFrameV2 &item) {
  FByteswapper(item.frame);
  FByteswapper(item.dataType);
  FByteswapper(item.fdata);
  FByteswapper(item.type);
}

void FByteswapper(AnimEvent &item) {
  FByteswapper(item.runEventBit);
  FByteswapper(item.numFrames);
}

void FByteswapper(AnimEventV2 &item) {
  FByteswapper(item.frames);
  FByteswapper(item.numFrames);
  FByteswapper(item.eventHash);
  FByteswapper(item.dataType);
}

void FByteswapper(AnimEventGroupV2 &item) {
  FByteswapper(item.events);
  FByteswapper(item.numEvents);
  FByteswapper(item.groupHash);
}

void FByteswapper(AnimEventsHeaderV2 &item) {
  FByteswapper(item.eventGroups);
  FByteswapper(item.numGroups);
  FByteswapper(item.collectionHash);
}

template <>
void ProcessClass(AnimEventV2 &item, LMTConstructorProperties flags) {
  if (item.frames.Check(flags.ptrStore)) {
    return;
  }

  if (flags.swapEndian) {
    FByteswapper(item);
  }

  item.frames.Fixup(flags.base, flags.ptrStore);

  AnimEventFrameV2 *frames_ = item.frames;

  for (size_t e = 0; e < item.numFrames; e++) {
    FByteswapper(frames_[e]);
  }
}

template <>
void ProcessClass(AnimEventGroupV2 &item, LMTConstructorProperties flags) {
  if (item.events.Check(flags.ptrStore)) {
    return;
  }

  if (flags.swapEndian) {
    FByteswapper(item);
  }

  item.events.Fixup(flags.base, flags.ptrStore);

  AnimEventV2 *events_ = item.events;

  for (size_t e = 0; e < item.numEvents; e++) {
    ProcessClass(events_[e], flags);
  }
}

template <>
void ProcessClass(AnimEventsHeaderV2 &item, LMTConstructorProperties flags) {
  if (item.eventGroups.Check(flags.ptrStore)) {
    return;
  }

  if (flags.swapEndian) {
    FByteswapper(item);
  }

  item.eventGroups.Fixup(flags.base, flags.ptrStore);

  AnimEventGroupV2 *groups = item.eventGroups;

  for (size_t e = 0; e < item.numGroups; e++) {
    ProcessClass(groups[e], flags);
  }
}

struct LMTAnimationEventV2MidInterface : LMTAnimationEventV2 {
  AnimEventsHeaderV2 *header;
  LMTAnimationEventV2MidInterface(AnimEventsHeaderV2 *hdr) : header(hdr) {}

  uint32 GetHash() const override { return header->collectionHash; }
  uint32 GetGroupHash(size_t groupID) const override {
    AnimEventGroupV2 *groups_ = header->eventGroups;
    return groups_[groupID].groupHash;
  }
};

struct LMTAnimationEventMidInterface : LMTAnimationEventInterface {
  clgen::AnimationEvent::Interface interface;
  std::optional<LMTAnimationEventV2MidInterface> v2;

  LMTAnimationEventMidInterface(clgen::LayoutLookup rules, char *data)
      : interface {
    data, rules
  } {
  }

  bool Is64bit() const { return interface.lookup.x64; }

  std::span<AnimEvent> GetFrames(size_t groupID) const {
    auto groupSpan = interface.Groups();

    if (interface.LayoutVersion() >= LMT56) {
      groupSpan = interface.GroupsLMT56();
    }

    auto group = groupSpan.at(groupID);

    return {group.Events(), group.NumEvents()};
  }

  std::span<uint16> GetRemaps(size_t groupID) const {
    auto groupSpan = interface.Groups();

    if (interface.LayoutVersion() >= LMT56) {
      groupSpan = interface.GroupsLMT56();
    }

    auto group = groupSpan.at(groupID);

    return group.EventRemaps();
  }

  EventVariant Get() const override {
    if (interface.LayoutVersion() >= LMT92) {
      return {(const LMTAnimationEventV2 *)(&v2)};
    }

    return {static_cast<const LMTAnimationEventV1 *>(this)};
  }

  EventCollection GetEvents(size_t groupID) const override {
    auto frames = GetFrames(groupID);
    auto remaps = GetRemaps(groupID);
    EventCollection result;
    uint32 curFrame = 0;
    using TriggerType = decltype(AnimEvent::runEventBit);
    const size_t numEvents = sizeof(TriggerType) * 8;

    for (auto &f : frames) {
      if (f.runEventBit) {
        std::vector<int16> events;
        for (size_t i = 0; i < numEvents; i++) {
          if (f.runEventBit & (TriggerType(1) << i)) {
            events.push_back(remaps[i]);
          }
        }

        result.emplace(curFrame / frameRate, std::move(events));
      }

      curFrame += f.numFrames;
    }

    return result;
  }

  size_t GetNumGroups() const override {
    if (v2) {
      return v2->header->numGroups;
    }

    if (interface.LayoutVersion() >= LMT56) {
      return interface.GroupsLMT56().count;
    }

    return interface.Groups().count;
  }
};

template <>
void ProcessClass(LMTAnimationEventMidInterface &item,
                  LMTConstructorProperties flags) {
  if (item.interface.LayoutVersion() >= LMT92) {
    auto ptr = item.interface.GroupsPtr();

    if (ptr.Check(flags.ptrStore)) {
      return;
    }

    if (flags.swapEndian) {
      clgen::EndianSwap(item.interface);
    }

    ptr.Fixup(flags.base, flags.ptrStore);
    ProcessClass(**ptr, flags);
    item.v2.emplace(*ptr);
    return;
  }

  auto groupSpan = item.interface.Groups();

  if (item.interface.LayoutVersion() >= LMT56) {
    groupSpan = item.interface.GroupsLMT56();
  }

  for (size_t gindex = 0; auto g : groupSpan) {
    if (g.EventsPtr().Check(flags.ptrStore)) {
      return;
    }

    if (flags.swapEndian) {
      clgen::EndianSwap(g);
    }

    g.EventsPtr().Fixup(flags.base, flags.ptrStore);

    if (flags.swapEndian) {
      for (auto &a : item.GetFrames(gindex++)) {
        FByteswapper(a);
      }
    }
  }
}

using ptr_type_ = std::unique_ptr<LMTAnimationEvent>;

ptr_type_ LMTAnimationEvent::Create(const LMTConstructorProperties &props) {
  auto instance = std::make_unique<LMTAnimationEventMidInterface>(
      clgen::LayoutLookup{static_cast<uint8>(props.version),
                          props.arch == LMTArchType::X64, false},
      static_cast<char *>(props.dataStart));

  ProcessClass(*instance, props);

  return instance;
}
