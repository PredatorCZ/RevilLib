// This file has been automatically generated. Do not modify.
#include "lmt.inl"
namespace clgen::FloatTrack {
enum Members {
  componentRemaps,
  frames,
  numFloats,
  _count_,
};
static const std::set<ClassData<_count_>> LAYOUTS {
  {{{{LMT22, LMT95, 8, 0}}, 16}, {0, 8, 4}, {0x2c}},
  {{{{LMT22, LMT95, 4, 0}}, 12}, {0, 8, 4}, {0x28}}
};
struct Interface {
  Interface(char *data_, LayoutLookup layout_): data{data_}, layout{GetLayout(LAYOUTS, {layout_, {LookupFlag::Ptr}})}, lookup{layout_} {}
  Interface(const Interface&) = default;
  Interface(Interface&&) = default;
  Interface &operator=(const Interface&) = default;
  Interface &operator=(Interface&&) = default;
  uint16 LayoutVersion() const { return lookup.version; }
  std::span<FloatTrackComponentRemap> ComponentRemaps() const { return m(componentRemaps) == -1 ? std::span<FloatTrackComponentRemap>{} : std::span<FloatTrackComponentRemap>{reinterpret_cast<FloatTrackComponentRemap*>(data + m(componentRemaps)), 4}; }
  uint32 NumFloats() const { return m(numFloats) == -1 ? uint32{} : *reinterpret_cast<uint32*>(data + m(numFloats)); }
  Pointer<FloatFrame> FramesPtr() {
    int16 off = m(frames); if (off == -1) return {nullptr, lookup};
    return {data + off, lookup};
  }
  FloatFrame *Frames() {
    int16 off = m(frames); if (off == -1) return nullptr;
    if (layout->ptrSize == 8) return *reinterpret_cast<FloatFrame**>(data + off);
    return *reinterpret_cast<es::PointerX86<FloatFrame>*>(data + off);
  }
  const FloatFrame *Frames() const {
    int16 off = m(frames); if (off == -1) return nullptr;
    if (layout->ptrSize == 8) return *reinterpret_cast<FloatFrame**>(data + off);
    return *reinterpret_cast<es::PointerX86<FloatFrame>*>(data + off);
  }
  void NumFloats(uint32 value) { if (m(numFloats) >= 0) *reinterpret_cast<uint32*>(data + m(numFloats)) = value; }


  int16 m(uint32 id) const { return layout->vtable[id]; }
  char *data;
  const ClassData<_count_> *layout;
  LayoutLookup lookup;
};
}
namespace clgen::FloatTracks {
enum Members {
  groups,
  _count_,
};
static const std::set<ClassData<_count_>> LAYOUTS {
  {{{{LMT22, LMT95, 8, 0}}, 64}, {0}, {0x0}},
  {{{{LMT22, LMT95, 4, 0}}, 48}, {0}, {0x0}}
};
struct Interface {
  Interface(char *data_, LayoutLookup layout_): data{data_}, layout{GetLayout(LAYOUTS, {layout_, {LookupFlag::Ptr}})}, lookup{layout_} {}
  Interface(const Interface&) = default;
  Interface(Interface&&) = default;
  Interface &operator=(const Interface&) = default;
  Interface &operator=(Interface&&) = default;
  uint16 LayoutVersion() const { return lookup.version; }
  LayoutedSpan<FloatTrack::Interface> Groups() const { return {{m(groups) == -1 ? nullptr : data + m(groups), lookup}, 4 }; }
  

  int16 m(uint32 id) const { return layout->vtable[id]; }
  char *data;
  const ClassData<_count_> *layout;
  LayoutLookup lookup;
};
}
