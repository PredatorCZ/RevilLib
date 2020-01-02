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

#include "LMTBoneTrack.h"
#include "LMTFixupStorage.h"
#include "datas/deleter_hybrid.hpp"
#include "datas/disabler.hpp"
#include "datas/reflectorRegistry.hpp"
#include <memory>
#include <unordered_map>

REFLECTOR_CREATE(TrackType_e, ENUM, 2, CLASS, 8, LocalRotation, LocalPosition,
                 LocalScale, AbsoluteRotation, AbsolutePosition);

REFLECTOR_CREATE(TrackV1BufferTypes, ENUM, 2, CLASS, 8, SingleVector3 = 1,
                 SinglePositionVector3, SingleRotationQuat3 = 4, HermiteVector3,
                 SphericalRotation, LinearVector3 = 9);

REFLECTOR_CREATE(TrackV1_5BufferTypes, ENUM, 2, CLASS, 8, SingleVector3 = 1,
                 SinglePositionVector3, SingleRotationQuat3 = 4, HermiteVector3,
                 LinearRotationQuat4_14bit, LinearVector3 = 9)

REFLECTOR_CREATE(TrackV2BufferTypes, ENUM, 2, CLASS, 8, SingleVector3 = 1,
                 SingleRotationQuat3, LinearVector3, BiLinearVector3_16bit,
                 BiLinearVector3_8bit, LinearRotationQuat4_14bit,
                 BiLinearRotationQuat4_7bit, BiLinearRotationQuatXW_14bit = 11,
                 BiLinearRotationQuatYW_14bit, BiLinearRotationQuatZW_14bit,
                 BiLinearRotationQuat4_11bit, BiLinearRotationQuat4_9bit)

template <template <class C> class PtrType> struct TrackV0 {
  DECLARE_REFLECTOR;

  static const int SUBVERSION = 0;
  static const int POINTERS[];

  TrackV1BufferTypes compression;
  TrackType_e trackType;
  uchar boneType;
  uchar boneID;
  float weight;
  uint bufferSize;
  PtrType<char> bufferOffset;

  void noExtremes();
  void noMHBone();
  void noRefFrame();

  void SwapEndian() {
    FByteswapper(weight);
    FByteswapper(bufferSize);
  }

  ES_FORCEINLINE void Fixup(char *masterBuffer, bool swapEndian) {
    bufferOffset.Fixup(masterBuffer, swapEndian);

    if (swapEndian)
      SwapEndian();
  }
};

template <class C> struct BufferVersion {};
template <> struct BufferVersion<TrackV1BufferTypes> {
  static constexpr int value = 0;
};
template <> struct BufferVersion<TrackV1_5BufferTypes> {
  static constexpr int value = 1;
};

template <template <class C> class PtrType, class BufferType> struct TrackV1 {
  DECLARE_REFLECTOR;

  static const int SUBVERSION = BufferVersion<BufferType>::value;
  static const int POINTERS[];

  BufferType compression;
  TrackType_e trackType;
  uchar boneType;
  uchar boneID;
  float weight;
  uint bufferSize;
  PtrType<char> bufferOffset;
  Vector4 referenceData;

  void noExtremes();
  void noMHBone();

  void SwapEndian() {
    FByteswapper(weight);
    FByteswapper(bufferSize);
    FByteswapper(referenceData);
  }

  ES_FORCEINLINE void Fixup(char *masterBuffer, bool swapEndian) {
    bufferOffset.Fixup(masterBuffer, swapEndian);

    if (swapEndian)
      SwapEndian();
  }
};

template <template <class C> class PtrType> struct TrackV2 {
  DECLARE_REFLECTOR;

  static const int SUBVERSION = 2;
  static const int POINTERS[];

  TrackV2BufferTypes compression;
  TrackType_e trackType;
  uchar boneType;
  uchar boneID;
  float weight;
  int bufferSize;
  PtrType<char> bufferOffset;
  Vector4 referenceData;
  PtrType<TrackMinMax> extremes;

  void noMHBone();

  void SwapEndian() {
    FByteswapper(reinterpret_cast<int &>(compression));
    FByteswapper(weight);
    FByteswapper(bufferSize);
    FByteswapper(referenceData);
  }

  ES_FORCEINLINE void Fixup(char *masterBuffer, bool swapEndian) {
    bufferOffset.Fixup(masterBuffer, swapEndian);
    extremes.Fixup(masterBuffer, swapEndian);

    if (swapEndian)
      SwapEndian();
  }
};

template <template <class C> class PtrType> struct TrackV3 {
  DECLARE_REFLECTOR;

  static const int SUBVERSION = 2;
  static const int POINTERS[];

  TrackV2BufferTypes compression;
  TrackType_e trackType;
  uchar boneType;
  uchar boneID2;
  int boneID;
  float weight;
  int bufferSize;
  PtrType<char> bufferOffset;
  Vector4 referenceData;
  PtrType<TrackMinMax> extremes;

  void SwapEndian() {
    FByteswapper(reinterpret_cast<int &>(compression));
    FByteswapper(boneID);
    FByteswapper(weight);
    FByteswapper(bufferSize);
    FByteswapper(referenceData);
  }

  ES_FORCEINLINE void Fixup(char *masterBuffer, bool swapEndian) {
    bufferOffset.Fixup(masterBuffer, swapEndian);
    extremes.Fixup(masterBuffer, swapEndian);

    if (swapEndian)
      SwapEndian();
  }
};

void LMTTrack_internal::SwapEndian() {
  if (controller)
    controller->SwapEndian();
}

int LMTTrack_internal::NumFrames() const {
  return controller->NumFrames() + useRefFrame;
}
bool LMTTrack_internal::LMTTrack_internal::IsCubic() const {
  return controller->IsCubic();
}
void LMTTrack_internal::GetTangents(Vector4A16 &inTangs, Vector4A16 &outTangs,
                                    int frame) const {
  if (useRefFrame && !frame) {
    inTangs = Vector4A16(*GetRefData());
    outTangs = inTangs;
    return;
  }

  controller->GetTangents(inTangs, outTangs, frame);
}
void LMTTrack_internal::Evaluate(Vector4A16 &out, int frame) const {
  if (useRefFrame && !frame) {
    out = Vector4A16(*GetRefData());
    return;
  }

  controller->Evaluate(out, frame - useRefFrame);

  if (minMax)
    out = minMax->max + minMax->min * out;
}
short LMTTrack_internal::GetFrame(int frame) const {
  if (useRefFrame && !frame) {
    return 0;
  }

  return controller->GetFrame(frame - useRefFrame) + useRefFrame;
}

void LMTTrack_internal::Interpolate(Vector4A16 &out, float time) const {
  float frameDelta = time * 60.f;
  const int frame = frameDelta;
  const int numCtrFrames = controller->NumFrames();

  if (!numCtrFrames && !useRefFrame)
    return;

  if ((!frame || !numCtrFrames) && useRefFrame) {
    Vector4A16 refFrame(*GetRefData());

    if (frameDelta < 0.0001f || !numCtrFrames) {
      out = refFrame;
    } else {
      controller->Evaluate(out, 0);
      frameDelta -= frame;
      out = refFrame + (out - refFrame) * frameDelta;
    }
  } else {
    const int ctrFrame = frame - useRefFrame;
    const int maxFrame = controller->GetFrame(numCtrFrames - 1);

    if (ctrFrame >= maxFrame) {
      Evaluate(out, numCtrFrames - 1);
    } else {
      for (int f = 1; f < numCtrFrames; f++) {
        short cFrame = controller->GetFrame(f);

        if (cFrame > ctrFrame) {
          const float boundFrame = cFrame;
          const float prevFrame = controller->GetFrame(f - 1);

          frameDelta = (prevFrame - frameDelta) / (prevFrame - boundFrame);

          controller->Interpolate(out, f - 1, frameDelta, minMax.get());
          break;
        }
      }
    }
  }
}

template <class C> class LMTTrackShared : public LMTTrack_internal {
  ADD_DISABLERS(C, noExtremes, noMHBone, noRefFrame);

  enabledFunction(noExtremes, TrackMinMax *)
      GetTrackExtremes(char *masterBuffer) {
    return reinterpret_cast<TrackMinMax *>(
        data->extremes.GetData(masterBuffer));
  }

  disabledFunction(noExtremes, TrackMinMax *) GetTrackExtremes(char *) {
    return nullptr;
  }

  int UseRefFrame() const {
    return !decltype(detectornoRefFrame<this_type>(nullptr))::value;
  }

public:
  typedef C value_type;

  std::unique_ptr<C, std::deleter_hybrid> data;

  LMTTrackShared() : data(new C) { useRefFrame = UseRefFrame(); }
  LMTTrackShared(C *_data, char *masterBuffer, bool swapEndian)
      : data(_data, false) {
    useRefFrame = UseRefFrame();
    data->Fixup(masterBuffer, swapEndian);

    if (CreateController())
      controller->Assign(data->bufferOffset.GetData(masterBuffer),
                         data->bufferSize);

    minMax = MinMaxPtr(GetTrackExtremes(masterBuffer), false);

    if (swapEndian)
      SwapEndian();
  }

  bool CreateController() override {
    controller = LMTTrackControllerPtr(LMTTrackController::CreateCodec(
        static_cast<int>(data->compression), C::SUBVERSION));

    return static_cast<bool>(controller);
  }

  TrackType GetTrackType() const noexcept override {
    return static_cast<TrackType>(data->trackType);
  }

  void SetTrackType(TrackType type) noexcept override {
    data->trackType = static_cast<TrackType_e>(type);
  }

  int Stride() const override { return static_cast<int>(sizeof(C)); }

  int AnimatedBoneID() const noexcept override { return _BoneID(); }

  void BoneID(int boneID) noexcept override { _BoneID(boneID); }

  const Vector4 *GetRefData() const override { return _RefData(); }

  enabledFunction(noRefFrame, const Vector4 *) _RefData() const {
    return &data->referenceData;
  }

  disabledFunction(noRefFrame, const Vector4 *) _RefData() const {
    return nullptr;
  }

  enabledFunction(noMHBone, int) _BoneID() const noexcept {
    return data->boneID;
  }

  disabledFunction(noMHBone, int) _BoneID() const noexcept {
    return data->boneID == 0xff ? -1 : data->boneID;
  }

  enabledFunction(noMHBone, void) _BoneID(int boneID) noexcept {
    data->boneID = boneID;
  }

  disabledFunction(noMHBone, void) _BoneID(int boneID) noexcept {
    data->boneID = boneID == -1 ? 0xff : boneID;
  }

  // research this
  enabledFunction(noMHBone, int) MirroredBoneID() const noexcept {
    return data->boneID2;
  }

  disabledFunction(noMHBone, int) MirroredBoneID() const noexcept { return -1; }

  enabledFunction(noMHBone, void) MirroredBoneID(int boneID) noexcept {
    data->boneID2 = boneID;
  }

  disabledFunction(noMHBone, void) MirroredBoneID(int boneID) noexcept {}

  bool UseTrackExtremes() const override {
    return !decltype(detectornoExtremes<this_type>(nullptr))::value;
  }

  void _ToXML(pugi::xml_node &node) const override {
    ReflectorWrapConst<C> reflt(data.get());
    reflt.ToXML(node, false);
  }

  void _FromXML(pugi::xml_node &node) override {
    ReflectorWrap<C> reflt(data.get());
    reflt.FromXML(node, false);
  }

  void Save(BinWritter *wr, LMTFixupStorage &storage) const override {
    const size_t cOff = wr->Tell();

    wr->Write(*data);

    for (auto &p : data->POINTERS)
      storage.SaveFrom(cOff + p);
  }
};

REGISTER_ENUMS(TrackV1BufferTypes, TrackV1_5BufferTypes, TrackV2BufferTypes,
               TrackType_e);

LMTTrack_internal::LMTTrack_internal() : controller(nullptr), minMax(nullptr) {
  RegisterLocalEnums();
}

typedef TrackV0<PointerX86> TrackV0PointerX86;
template <>
const int TrackV0PointerX86::POINTERS[] = {offsetof(TrackV0, bufferOffset)};
REFLECTOR_CREATE(TrackV0PointerX86, 2, VARNAMES, TEMPLATE, compression,
                 trackType, boneType, boneID, weight);

typedef TrackV0<PointerX64> TrackV0PointerX64;
template <>
const int TrackV0PointerX64::POINTERS[] = {offsetof(TrackV0, bufferOffset)};
REFLECTOR_CREATE(TrackV0PointerX64, 2, VARNAMES, TEMPLATE, compression,
                 trackType, boneType, boneID, weight);

typedef TrackV1<PointerX86, TrackV1BufferTypes>
    TrackV1TrackV1BufferTypesPointerX86;
template <>
const int TrackV1TrackV1BufferTypesPointerX86::POINTERS[] = {
    offsetof(TrackV1, bufferOffset)};
REFLECTOR_CREATE(TrackV1TrackV1BufferTypesPointerX86, 2, VARNAMES, TEMPLATE,
                 compression, trackType, boneType, boneID, weight,
                 referenceData);

typedef TrackV1<PointerX64, TrackV1BufferTypes>
    TrackV1TrackV1BufferTypesPointerX64;
template <>
const int TrackV1TrackV1BufferTypesPointerX64::POINTERS[] = {
    offsetof(TrackV1, bufferOffset)};
REFLECTOR_CREATE(TrackV1TrackV1BufferTypesPointerX64, 2, VARNAMES, TEMPLATE,
                 compression, trackType, boneType, boneID, weight,
                 referenceData);

typedef TrackV1<PointerX86, TrackV1_5BufferTypes>
    TrackV1TrackV1_5BufferTypesPointerX86;
template <>
const int TrackV1TrackV1_5BufferTypesPointerX86::POINTERS[] = {
    offsetof(TrackV1, bufferOffset)};
REFLECTOR_CREATE(TrackV1TrackV1_5BufferTypesPointerX86, 2, VARNAMES, TEMPLATE,
                 compression, trackType, boneType, boneID, weight,
                 referenceData);

typedef TrackV1<PointerX64, TrackV1_5BufferTypes>
    TrackV1TrackV1_5BufferTypesPointerX64;
template <>
const int TrackV1TrackV1_5BufferTypesPointerX64::POINTERS[] = {
    offsetof(TrackV1, bufferOffset)};
REFLECTOR_CREATE(TrackV1TrackV1_5BufferTypesPointerX64, 2, VARNAMES, TEMPLATE,
                 compression, trackType, boneType, boneID, weight,
                 referenceData);

typedef TrackV2<PointerX86> TrackV2PointerX86;
template <>
const int TrackV2PointerX86::POINTERS[] = {offsetof(TrackV2, bufferOffset),
                                           offsetof(TrackV2, extremes)};
REFLECTOR_CREATE(TrackV2PointerX86, 2, VARNAMES, TEMPLATE, compression,
                 trackType, boneType, boneID, weight, referenceData);

typedef TrackV2<PointerX64> TrackV2PointerX64;
template <>
const int TrackV2PointerX64::POINTERS[] = {offsetof(TrackV2, bufferOffset),
                                           offsetof(TrackV2, extremes)};
REFLECTOR_CREATE(TrackV2PointerX64, 2, VARNAMES, TEMPLATE, compression,
                 trackType, boneType, boneID, weight, referenceData);

typedef TrackV3<PointerX86> TrackV3PointerX86;
template <>
const int TrackV3PointerX86::POINTERS[] = {offsetof(TrackV3, bufferOffset),
                                           offsetof(TrackV3, extremes)};
REFLECTOR_CREATE(TrackV3PointerX86, 2, VARNAMES, TEMPLATE, compression,
                 trackType, boneType, boneID, boneID2, weight, referenceData);

typedef TrackV3<PointerX64> TrackV3PointerX64;
template <>
const int TrackV3PointerX64::POINTERS[] = {offsetof(TrackV3, bufferOffset),
                                           offsetof(TrackV3, extremes)};
REFLECTOR_CREATE(TrackV3PointerX64, 2, VARNAMES, TEMPLATE, compression,
                 trackType, boneType, boneID, boneID2, weight, referenceData);

template <class C> static LMTTrack *_creattorBase() {
  return new LMTTrackShared<C>;
}

template <class C> static LMTTrack *_creator(void *ptr, char *buff, bool endi) {
  return new LMTTrackShared<C>(static_cast<C *>(ptr), buff, endi);
}

static const std::unordered_map<short, LMTTrack *(*)()> trackRegistry = {
    {0x08, _creattorBase<TrackV0PointerX64>},
    {0x04, _creattorBase<TrackV0PointerX86>},
    {0x108, _creattorBase<TrackV1TrackV1BufferTypesPointerX64>},
    {0x104, _creattorBase<TrackV1TrackV1BufferTypesPointerX86>},
    {0x208, _creattorBase<TrackV1TrackV1_5BufferTypesPointerX64>},
    {0x204, _creattorBase<TrackV1TrackV1_5BufferTypesPointerX86>},
    {0x308, _creattorBase<TrackV2PointerX64>},
    {0x304, _creattorBase<TrackV2PointerX86>},
    {0x408, _creattorBase<TrackV3PointerX64>},
    {0x404, _creattorBase<TrackV3PointerX86>}};

static const std::unordered_map<short, LMTTrack *(*)(void *, char *, bool)>
    trackRegistryLink = {
        {0x08, _creator<TrackV0PointerX64>},
        {0x04, _creator<TrackV0PointerX86>},
        {0x108, _creator<TrackV1TrackV1BufferTypesPointerX64>},
        {0x104, _creator<TrackV1TrackV1BufferTypesPointerX86>},
        {0x208, _creator<TrackV1TrackV1_5BufferTypesPointerX64>},
        {0x204, _creator<TrackV1TrackV1_5BufferTypesPointerX86>},
        {0x308, _creator<TrackV2PointerX64>},
        {0x304, _creator<TrackV2PointerX86>},
        {0x408, _creator<TrackV3PointerX64>},
        {0x404, _creator<TrackV3PointerX86>}};

LMTTrack *LMTTrack::Create(const LMTConstructorProperties &props) {
  short item = reinterpret_cast<const short &>(props);

  if (!trackRegistry.count(item))
    return nullptr;

  if (props.dataStart)
    return trackRegistryLink.at(item)(props.dataStart, props.masterBuffer,
                                      props.swappedEndian);
  else
    return trackRegistry.at(item)();
}