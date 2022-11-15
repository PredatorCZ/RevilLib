// This file has been automatically generated. Do not modify.
#include "lmt.inl"
namespace clgen::AnimEvents {
enum Members {
  eventRemaps,
  events,
  numEvents,
  _count_,
};
static const std::set<ClassData<_count_>> LAYOUTS {
  {{{{LMT22, LMT95, 8, 0}}, 80}, {0, 72, 64}, {0x2c}},
  {{{{LMT22, LMT95, 4, 0}}, 72}, {0, 68, 64}, {0x28}}
};
struct Interface {
  Interface(char *data_, LayoutLookup layout_): data{data_}, layout{GetLayout(LAYOUTS, {layout_, {LookupFlag::Ptr}})}, lookup{layout_} {}
  Interface(const Interface&) = default;
  Interface(Interface&&) = default;
  Interface &operator=(const Interface&) = default;
  Interface &operator=(Interface&&) = default;
  uint16 LayoutVersion() const { return lookup.version; }
  std::span<uint16> EventRemaps() const { return m(eventRemaps) == -1 ? std::span<uint16>{} : std::span<uint16>{reinterpret_cast<uint16*>(data + m(eventRemaps)), 32}; }
  uint32 NumEvents() const { return m(numEvents) == -1 ? uint32{} : *reinterpret_cast<uint32*>(data + m(numEvents)); }
  Pointer<AnimEvent> EventsPtr() {
    int16 off = m(events); if (off == -1) return {nullptr, lookup};
    return {data + off, lookup};
  }
  AnimEvent *Events() {
    int16 off = m(events); if (off == -1) return nullptr;
    if (layout->ptrSize == 8) return *reinterpret_cast<AnimEvent**>(data + off);
    return *reinterpret_cast<es::PointerX86<AnimEvent>*>(data + off);
  }
  const AnimEvent *Events() const {
    int16 off = m(events); if (off == -1) return nullptr;
    if (layout->ptrSize == 8) return *reinterpret_cast<AnimEvent**>(data + off);
    return *reinterpret_cast<es::PointerX86<AnimEvent>*>(data + off);
  }
  void NumEvents(uint32 value) { if (m(numEvents) >= 0) *reinterpret_cast<uint32*>(data + m(numEvents)) = value; }


  int16 m(uint32 id) const { return layout->vtable[id]; }
  char *data;
  const ClassData<_count_> *layout;
  LayoutLookup lookup;
};
}
namespace clgen::AnimationEvent {
enum Members {
  groups,
  _count_,
};
static const std::set<ClassData<_count_>> LAYOUTS {
  {{{{LMT22, LMT51, 8, 0}}, 160}, {0}, {0x0}},
  {{{{LMT56, LMT67, 8, 0}}, 320}, {0}, {0x0}},
  {{{{LMT92, LMT95, 8, 0}}, 8}, {0}, {0x3}},
  {{{{LMT22, LMT51, 4, 0}}, 144}, {0}, {0x0}},
  {{{{LMT56, LMT67, 4, 0}}, 288}, {0}, {0x0}},
  {{{{LMT92, LMT95, 4, 0}}, 4}, {0}, {0x2}}
};
struct Interface {
  Interface(char *data_, LayoutLookup layout_): data{data_}, layout{GetLayout(LAYOUTS, {layout_, {LookupFlag::Ptr}})}, lookup{layout_} {}
  Interface(const Interface&) = default;
  Interface(Interface&&) = default;
  Interface &operator=(const Interface&) = default;
  Interface &operator=(Interface&&) = default;
  uint16 LayoutVersion() const { return lookup.version; }
  LayoutedSpan<AnimEvents::Interface> Groups() const { return {{m(groups) == -1 ? nullptr : data + m(groups), lookup}, 2 }; }
  LayoutedSpan<AnimEvents::Interface> GroupsLMT56() const { return {{m(groups) == -1 ? nullptr : data + m(groups), lookup}, 4 }; }
  Pointer<AnimEventsHeaderV2> GroupsPtr() {
    int16 off = m(groups); if (off == -1) return {nullptr, lookup};
    return {data + off, lookup};
  }
  AnimEventsHeaderV2 *GroupsLMT92() {
    int16 off = m(groups); if (off == -1) return nullptr;
    if (layout->ptrSize == 8) return *reinterpret_cast<AnimEventsHeaderV2**>(data + off);
    return *reinterpret_cast<es::PointerX86<AnimEventsHeaderV2>*>(data + off);
  }
  const AnimEventsHeaderV2 *GroupsLMT92() const {
    int16 off = m(groups); if (off == -1) return nullptr;
    if (layout->ptrSize == 8) return *reinterpret_cast<AnimEventsHeaderV2**>(data + off);
    return *reinterpret_cast<es::PointerX86<AnimEventsHeaderV2>*>(data + off);
  }
  

  int16 m(uint32 id) const { return layout->vtable[id]; }
  char *data;
  const ClassData<_count_> *layout;
  LayoutLookup lookup;
};
}
