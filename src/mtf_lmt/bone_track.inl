// This file has been automatically generated. Do not modify.
#include "lmt.inl"
namespace clgen::BoneTrack {
enum Members {
  boneID,
  boneID2,
  boneType,
  buffer,
  bufferSize,
  compression,
  extremes,
  referenceData,
  trackType,
  weight,
  _count_,
};
static const std::set<ClassData<_count_>> LAYOUTS {
  {{{{LMT22, LMT22, 8, 0}}, 24}, {3, -1, 2, 16, 8, 0, -1, -1, 1, 4}, {0x2c0, 0x8}},
  {{{{LMT40, LMT51, 8, 0}}, 40}, {3, -1, 2, 16, 8, 0, -1, 24, 1, 4}, {0x2c0, 0x8}},
  {{{{LMT56, LMT67, 8, 0}}, 48}, {3, -1, 2, 16, 8, 0, 40, 24, 1, 4}, {0x32c0, 0x8}},
  {{{{LMT92, LMT95, 8, 0}}, 48}, {3, 4, 2, 16, 12, 0, 40, 24, 1, 8}, {0x32c8, 0x8}},
  {{{{LMT22, LMT22, 4, 0}}, 16}, {3, -1, 2, 12, 8, 0, -1, -1, 1, 4}, {0x280, 0x8}},
  {{{{LMT40, LMT51, 4, 0}}, 32}, {3, -1, 2, 12, 8, 0, -1, 16, 1, 4}, {0x280, 0x8}},
  {{{{LMT56, LMT67, 4, 0}}, 36}, {3, -1, 2, 12, 8, 0, 32, 16, 1, 4}, {0x2280, 0x8}},
  {{{{LMT92, LMT95, 4, 0}}, 40}, {3, 4, 2, 16, 12, 0, 36, 20, 1, 8}, {0x2288, 0x8}}
};
struct Interface {
  Interface(char *data_, LayoutLookup layout_): data{data_}, layout{GetLayout(LAYOUTS, {layout_, {LookupFlag::Ptr}})}, lookup{layout_} {}
  Interface(const Interface&) = default;
  Interface(Interface&&) = default;
  Interface &operator=(const Interface&) = default;
  Interface &operator=(Interface&&) = default;
  uint16 LayoutVersion() const { return lookup.version; }
  TrackV1BufferTypes Compression() const { return m(compression) == -1 ? TrackV1BufferTypes{} : *reinterpret_cast<TrackV1BufferTypes*>(data + m(compression)); }
  TrackV1_5BufferTypes CompressionLMT51() const { return m(compression) == -1 ? TrackV1_5BufferTypes{} : *reinterpret_cast<TrackV1_5BufferTypes*>(data + m(compression)); }
  TrackV2BufferTypes CompressionLMT56() const { return m(compression) == -1 ? TrackV2BufferTypes{} : *reinterpret_cast<TrackV2BufferTypes*>(data + m(compression)); }
  TrackType_er TrackType() const { return m(trackType) == -1 ? TrackType_er{} : *reinterpret_cast<TrackType_er*>(data + m(trackType)); }
  uint8 BoneType() const { return m(boneType) == -1 ? uint8{} : *reinterpret_cast<uint8*>(data + m(boneType)); }
  uint8 BoneID() const { return m(boneID) == -1 ? uint8{} : *reinterpret_cast<uint8*>(data + m(boneID)); }
  float Weight() const { return m(weight) == -1 ? float{} : *reinterpret_cast<float*>(data + m(weight)); }
  uint32 BufferSize() const { return m(bufferSize) == -1 ? uint32{} : *reinterpret_cast<uint32*>(data + m(bufferSize)); }
  Pointer<char> BufferPtr() {
    int16 off = m(buffer); if (off == -1) return {nullptr, lookup};
    return {data + off, lookup};
  }
  char *Buffer() {
    int16 off = m(buffer); if (off == -1) return nullptr;
    if (layout->ptrSize == 8) return *reinterpret_cast<char**>(data + off);
    return *reinterpret_cast<es::PointerX86<char>*>(data + off);
  }
  const char *Buffer() const {
    int16 off = m(buffer); if (off == -1) return nullptr;
    if (layout->ptrSize == 8) return *reinterpret_cast<char**>(data + off);
    return *reinterpret_cast<es::PointerX86<char>*>(data + off);
  }
  Vector4 ReferenceData() const { return m(referenceData) == -1 ? Vector4{} : *reinterpret_cast<Vector4*>(data + m(referenceData)); }
  Pointer<TrackMinMax> ExtremesPtr() {
    int16 off = m(extremes); if (off == -1) return {nullptr, lookup};
    return {data + off, lookup};
  }
  TrackMinMax *Extremes() {
    int16 off = m(extremes); if (off == -1) return nullptr;
    if (layout->ptrSize == 8) return *reinterpret_cast<TrackMinMax**>(data + off);
    return *reinterpret_cast<es::PointerX86<TrackMinMax>*>(data + off);
  }
  const TrackMinMax *Extremes() const {
    int16 off = m(extremes); if (off == -1) return nullptr;
    if (layout->ptrSize == 8) return *reinterpret_cast<TrackMinMax**>(data + off);
    return *reinterpret_cast<es::PointerX86<TrackMinMax>*>(data + off);
  }
  int32 BoneID2() const { return m(boneID2) == -1 ? int32{} : *reinterpret_cast<int32*>(data + m(boneID2)); }
  void Compression(TrackV1BufferTypes value) { if (m(compression) >= 0) *reinterpret_cast<TrackV1BufferTypes*>(data + m(compression)) = value; }
  void CompressionLMT51(TrackV1_5BufferTypes value) { if (m(compression) >= 0) *reinterpret_cast<TrackV1_5BufferTypes*>(data + m(compression)) = value; }
  void CompressionLMT56(TrackV2BufferTypes value) { if (m(compression) >= 0) *reinterpret_cast<TrackV2BufferTypes*>(data + m(compression)) = value; }
  void TrackType(TrackType_er value) { if (m(trackType) >= 0) *reinterpret_cast<TrackType_er*>(data + m(trackType)) = value; }
  void BoneType(uint8 value) { if (m(boneType) >= 0) *reinterpret_cast<uint8*>(data + m(boneType)) = value; }
  void BoneID(uint8 value) { if (m(boneID) >= 0) *reinterpret_cast<uint8*>(data + m(boneID)) = value; }
  void Weight(float value) { if (m(weight) >= 0) *reinterpret_cast<float*>(data + m(weight)) = value; }
  void BufferSize(uint32 value) { if (m(bufferSize) >= 0) *reinterpret_cast<uint32*>(data + m(bufferSize)) = value; }
  void ReferenceData(Vector4 value) { if (m(referenceData) >= 0) *reinterpret_cast<Vector4*>(data + m(referenceData)) = value; }
  void BoneID2(int32 value) { if (m(boneID2) >= 0) *reinterpret_cast<int32*>(data + m(boneID2)) = value; }


  int16 m(uint32 id) const { return layout->vtable[id]; }
  char *data;
  const ClassData<_count_> *layout;
  LayoutLookup lookup;
};
}
