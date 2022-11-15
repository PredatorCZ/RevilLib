// This file has been automatically generated. Do not modify.
#include "event.inl"
#include "float_track.inl"
namespace clgen::Animation {
enum Members {
  endFrameAdditiveScenePosition,
  endFrameAdditiveSceneRotation,
  events,
  flags,
  floats,
  loopFrame,
  nullPtr,
  numFrames,
  numTracks,
  tracks,
  _count_,
};
static const std::set<ClassData<_count_>> LAYOUTS {
  {{{{LMT22, LMT22, 8, 0}}, 208}, {32, -1, 48, -1, -1, 16, -1, 12, 8, 0}, {0x8800, 0xe}},
  {{{{LMT40, LMT51, 8, 0}}, 224}, {32, 48, 64, -1, -1, 16, -1, 12, 8, 0}, {0x8800, 0xe}},
  {{{{LMT56, LMT57, 8, 0}}, 384}, {32, 48, 64, -1, -1, 16, -1, 12, 8, 0}, {0x8800, 0xe}},
  {{{{LMT66, LMT67, 8, 0}}, 96}, {32, 64, 80, 48, 88, 16, -1, 12, 8, 0}, {0x8b30, 0xe}},
  {{{{LMT92, LMT95, 8, 0}}, 112}, {32, 80, 96, 48, -1, 16, 56, 12, 8, 0}, {0x8830, 0xe}},
  {{{{LMT22, LMT22, 4, 0}}, 176}, {16, -1, 32, -1, -1, 12, -1, 8, 4, 0}, {0x8800, 0xa}},
  {{{{LMT40, LMT51, 4, 0}}, 192}, {16, 32, 48, -1, -1, 12, -1, 8, 4, 0}, {0x8800, 0xa}},
  {{{{LMT56, LMT57, 4, 0}}, 336}, {16, 32, 48, -1, -1, 12, -1, 8, 4, 0}, {0x8800, 0xa}},
  {{{{LMT66, LMT67, 4, 0}}, 80}, {16, 48, 64, 32, 68, 12, -1, 8, 4, 0}, {0x8a20, 0xa}},
  {{{{LMT92, LMT95, 4, 0}}, 80}, {16, 48, 64, 32, -1, 12, 36, 8, 4, 0}, {0x8820, 0xa}}
};
struct Interface {
  Interface(char *data_, LayoutLookup layout_): data{data_}, layout{GetLayout(LAYOUTS, {layout_, {LookupFlag::Ptr}})}, lookup{layout_} {}
  Interface(const Interface&) = default;
  Interface(Interface&&) = default;
  Interface &operator=(const Interface&) = default;
  Interface &operator=(Interface&&) = default;
  uint16 LayoutVersion() const { return lookup.version; }
  Pointer<char> TracksPtr() {
    int16 off = m(tracks); if (off == -1) return {nullptr, lookup};
    return {data + off, lookup};
  }
  char *Tracks() {
    int16 off = m(tracks); if (off == -1) return nullptr;
    if (layout->ptrSize == 8) return *reinterpret_cast<char**>(data + off);
    return *reinterpret_cast<es::PointerX86<char>*>(data + off);
  }
  const char *Tracks() const {
    int16 off = m(tracks); if (off == -1) return nullptr;
    if (layout->ptrSize == 8) return *reinterpret_cast<char**>(data + off);
    return *reinterpret_cast<es::PointerX86<char>*>(data + off);
  }
  uint32 NumTracks() const { return m(numTracks) == -1 ? uint32{} : *reinterpret_cast<uint32*>(data + m(numTracks)); }
  uint32 NumFrames() const { return m(numFrames) == -1 ? uint32{} : *reinterpret_cast<uint32*>(data + m(numFrames)); }
  int32 LoopFrame() const { return m(loopFrame) == -1 ? int32{} : *reinterpret_cast<int32*>(data + m(loopFrame)); }
  Vector4A16 EndFrameAdditiveScenePosition() const { return m(endFrameAdditiveScenePosition) == -1 ? Vector4A16{} : *reinterpret_cast<Vector4A16*>(data + m(endFrameAdditiveScenePosition)); }
  AnimationEvent::Interface Events() const { return {m(events) == -1 ? nullptr : data + m(events), lookup}; }
  Pointer<AnimationEvent::Interface> EventsPtr() {
    int16 off = m(events); if (off == -1) return {nullptr, lookup};
    return {data + off, lookup};
  }
  Iterator<AnimationEvent::Interface> EventsLMT66() {
    int16 off = m(events); if (off == -1) return {nullptr, lookup};
    if (layout->ptrSize == 8) return {*reinterpret_cast<char**>(data + off), lookup};
    return {*reinterpret_cast<es::PointerX86<char>*>(data + off), lookup};
  }
  Iterator<AnimationEvent::Interface> EventsLMT66() const {
    int16 off = m(events); if (off == -1) return {nullptr, lookup};
    if (layout->ptrSize == 8) return {*reinterpret_cast<char**>(data + off), lookup};
    return {*reinterpret_cast<es::PointerX86<char>*>(data + off), lookup};
  }
  Vector4A16 EndFrameAdditiveSceneRotation() const { return m(endFrameAdditiveSceneRotation) == -1 ? Vector4A16{} : *reinterpret_cast<Vector4A16*>(data + m(endFrameAdditiveSceneRotation)); }
  es::Flags<AnimV2Flags> Flags() const { return m(flags) == -1 ? es::Flags<AnimV2Flags>{} : *reinterpret_cast<es::Flags<AnimV2Flags>*>(data + m(flags)); }
  Pointer<FloatTracks::Interface> FloatsPtr() {
    int16 off = m(floats); if (off == -1) return {nullptr, lookup};
    return {data + off, lookup};
  }
  Iterator<FloatTracks::Interface> Floats() {
    int16 off = m(floats); if (off == -1) return {nullptr, lookup};
    if (layout->ptrSize == 8) return {*reinterpret_cast<char**>(data + off), lookup};
    return {*reinterpret_cast<es::PointerX86<char>*>(data + off), lookup};
  }
  Iterator<FloatTracks::Interface> Floats() const {
    int16 off = m(floats); if (off == -1) return {nullptr, lookup};
    if (layout->ptrSize == 8) return {*reinterpret_cast<char**>(data + off), lookup};
    return {*reinterpret_cast<es::PointerX86<char>*>(data + off), lookup};
  }
  LayoutedSpan<Pointer<char>> NullPtr() const { return {{m(nullPtr) == -1 ? nullptr : data + m(nullPtr), lookup}, 2 }; }
  void NumTracks(uint32 value) { if (m(numTracks) >= 0) *reinterpret_cast<uint32*>(data + m(numTracks)) = value; }
  void NumFrames(uint32 value) { if (m(numFrames) >= 0) *reinterpret_cast<uint32*>(data + m(numFrames)) = value; }
  void LoopFrame(int32 value) { if (m(loopFrame) >= 0) *reinterpret_cast<int32*>(data + m(loopFrame)) = value; }
  void EndFrameAdditiveScenePosition(Vector4A16 value) { if (m(endFrameAdditiveScenePosition) >= 0) *reinterpret_cast<Vector4A16*>(data + m(endFrameAdditiveScenePosition)) = value; }
  void EndFrameAdditiveSceneRotation(Vector4A16 value) { if (m(endFrameAdditiveSceneRotation) >= 0) *reinterpret_cast<Vector4A16*>(data + m(endFrameAdditiveSceneRotation)) = value; }
  void Flags(es::Flags<AnimV2Flags> value) { if (m(flags) >= 0) *reinterpret_cast<es::Flags<AnimV2Flags>*>(data + m(flags)) = value; }


  int16 m(uint32 id) const { return layout->vtable[id]; }
  char *data;
  const ClassData<_count_> *layout;
  LayoutLookup lookup;
};
}
