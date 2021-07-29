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

#include "bone_track.hpp"
#include "datas/deleter_hybrid.hpp"
#include "datas/reflector_xml.hpp"
#include "fixup_storage.hpp"
#include "pugixml.hpp"

#include <map>
#include <memory>

MAKE_ENUM(ENUMSCOPE(class TrackType_er
                    : uint8, TrackType_er),
          EMEMBER(LocalRotation), EMEMBER(LocalPosition), EMEMBER(LocalScale),
          EMEMBER(AbsoluteRotation), EMEMBER(AbsolutePosition));

MAKE_ENUM(ENUMSCOPE(class TrackV1BufferTypes
                    : uint8, TrackV1BufferTypes),
          EMEMBERVAL(SingleVector3, 1), EMEMBER(SinglePositionVector3),
          EMEMBERVAL(SingleRotationQuat3, 4), EMEMBER(HermiteVector3),
          EMEMBER(SphericalRotation), EMEMBERVAL(LinearVector3, 9));

MAKE_ENUM(ENUMSCOPE(class TrackV1_5BufferTypes
                    : uint8, TrackV1_5BufferTypes),
          EMEMBERVAL(SingleVector3, 1), EMEMBER(SinglePositionVector3),
          EMEMBERVAL(SingleRotationQuat3, 4), EMEMBER(HermiteVector3),
          EMEMBER(LinearRotationQuat4_14bit), EMEMBERVAL(LinearVector3, 9));

MAKE_ENUM(ENUMSCOPE(class TrackV2BufferTypes
                    : uint8, TrackV2BufferTypes),
          EMEMBERVAL(SingleVector3, 1), EMEMBER(SingleRotationQuat3),
          EMEMBER(LinearVector3), EMEMBER(BiLinearVector3_16bit),
          EMEMBER(BiLinearVector3_8bit), EMEMBER(LinearRotationQuat4_14bit),
          EMEMBER(BiLinearRotationQuat4_7bit),
          EMEMBERVAL(BiLinearRotationQuatXW_14bit, 11),
          EMEMBER(BiLinearRotationQuatYW_14bit),
          EMEMBER(BiLinearRotationQuatZW_14bit),
          EMEMBER(BiLinearRotationQuat4_11bit),
          EMEMBER(BiLinearRotationQuat4_9bit))

template <template <class C> class PtrType> struct TrackV0 {
  static constexpr uint32 SUBVERSION = 0;
  static constexpr size_t NUMPOINTERS = 1;

  TrackV1BufferTypes compression;
  TrackType_er trackType;
  uint8 boneType;
  uint8 boneID;
  float weight;
  uint32 bufferSize;
  PtrType<char> bufferOffset;

  void SwapEndian() {
    FByteswapper(bufferOffset);
    FByteswapper(weight);
    FByteswapper(bufferSize);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    auto cb = [&] {
      if (swapEndian) {
        SwapEndian();
      }
    };

    if (!es::FixupPointersCB(masterBuffer, ptrStore, cb, bufferOffset)) {
      return;
    }
  }

  static const size_t *Pointers() {
    static const size_t ptrs[]{
        offsetof(TrackV0, bufferOffset),
    };

    return ptrs;
  }
};

template <class C> struct BufferVersion {};
template <> struct BufferVersion<TrackV1BufferTypes> {
  static constexpr uint32 value = 0;
};
template <> struct BufferVersion<TrackV1_5BufferTypes> {
  static constexpr uint32 value = 1;
};

template <template <class C> class PtrType, class BufferType> struct TrackV1 {
  static constexpr uint32 SUBVERSION = BufferVersion<BufferType>::value;
  static constexpr size_t NUMPOINTERS = 1;

  BufferType compression;
  TrackType_er trackType;
  uint8 boneType;
  uint8 boneID;
  float weight;
  uint32 bufferSize;
  PtrType<char> bufferOffset;
  Vector4 referenceData;

  void SwapEndian() {
    FByteswapper(weight);
    FByteswapper(bufferSize);
    FByteswapper(bufferOffset);
    FByteswapper(referenceData);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    auto cb = [&] {
      if (swapEndian) {
        SwapEndian();
      }
    };

    if (!es::FixupPointersCB(masterBuffer, ptrStore, cb, bufferOffset)) {
      return;
    }
  }

  static const size_t *Pointers() {
    static const size_t ptrs[]{
        offsetof(TrackV1, bufferOffset),
    };

    return ptrs;
  }
};

template <template <class C> class PtrType> struct TrackV2 {
  static constexpr uint32 SUBVERSION = 2;
  static constexpr size_t NUMPOINTERS = 2;

  TrackV2BufferTypes compression;
  TrackType_er trackType;
  uint8 boneType;
  uint8 boneID;
  float weight;
  uint32 bufferSize;
  PtrType<char> bufferOffset;
  Vector4 referenceData;
  PtrType<TrackMinMax> extremes;

  void SwapEndian() {
    FByteswapper(reinterpret_cast<uint32 &>(compression));
    FByteswapper(weight);
    FByteswapper(bufferSize);
    FByteswapper(bufferOffset);
    FByteswapper(referenceData);
    FByteswapper(extremes);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    auto cb = [&] {
      if (swapEndian) {
        SwapEndian();
      }
    };

    if (!es::FixupPointersCB(masterBuffer, ptrStore, cb, bufferOffset,
                             extremes)) {
      return;
    }
  }

  static const size_t *Pointers() {
    static const size_t ptrs[]{
        offsetof(TrackV2, bufferOffset),
        offsetof(TrackV2, extremes),
    };

    return ptrs;
  }
};

template <template <class C> class PtrType> struct TrackV3 {
  static constexpr uint32 SUBVERSION = 2;
  static constexpr size_t NUMPOINTERS = 2;

  TrackV2BufferTypes compression;
  TrackType_er trackType;
  uint8 boneType;
  uint8 boneID2;
  int32 boneID;
  float weight;
  uint32 bufferSize;
  PtrType<char> bufferOffset;
  Vector4 referenceData;
  PtrType<TrackMinMax> extremes;

  void SwapEndian() {
    FByteswapper(reinterpret_cast<uint32 &>(compression));
    FByteswapper(boneID);
    FByteswapper(weight);
    FByteswapper(bufferSize);
    FByteswapper(bufferOffset);
    FByteswapper(referenceData);
    FByteswapper(extremes);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    auto cb = [&] {
      if (swapEndian) {
        SwapEndian();
      }
    };

    if (!es::FixupPointersCB(masterBuffer, ptrStore, cb, bufferOffset,
                             extremes)) {
      return;
    }
  }

  static const size_t *Pointers() {
    static const size_t ptrs[]{
        offsetof(TrackV3, bufferOffset),
        offsetof(TrackV3, extremes),
    };

    return ptrs;
  }
};

size_t LMTTrack_internal::NumFrames() const {
  return controller->NumFrames() + useRefFrame;
}

bool LMTTrack_internal::LMTTrack_internal::IsCubic() const {
  return controller->IsCubic();
}

void LMTTrack_internal::GetTangents(Vector4A16 &inTangs, Vector4A16 &outTangs,
                                    size_t frame) const {
  if (useRefFrame && !frame) {
    inTangs = Vector4A16(*GetRefData());
    outTangs = inTangs;
    return;
  }

  controller->GetTangents(inTangs, outTangs, frame);
}

void LMTTrack_internal::Evaluate(Vector4A16 &out, size_t frame) const {
  if (useRefFrame && !frame) {
    out = Vector4A16(*GetRefData());
    return;
  }

  controller->Evaluate(out, frame - useRefFrame);

  if (useMinMax) {
    out = minMax.max + minMax.min * out;
  }
}

int32 LMTTrack_internal::GetFrame(size_t frame) const {
  if (useRefFrame && !frame) {
    return 0;
  }

  return controller->GetFrame(frame - useRefFrame) + useRefFrame;
}

void LMTTrack_internal::GetValue(Vector4A16 &out, float time) const {
  float frameDelta = time * frameRate;
  const int32 frame = static_cast<int32>(frameDelta);
  const size_t numCtrFrames = controller->NumFrames();

  if (!numCtrFrames && !useRefFrame) {
    return;
  }

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
    const int32 ctrFrame = frame - useRefFrame;
    const int32 maxFrame = controller->GetFrame(numCtrFrames - 1);

    if (ctrFrame >= maxFrame) {
      Evaluate(out, numCtrFrames - 1);
    } else {
      for (size_t f = 1; f < numCtrFrames; f++) {
        int32 cFrame = controller->GetFrame(f);

        if (cFrame > ctrFrame) {
          const float boundFrame = static_cast<float>(cFrame);
          const float prevFrame =
              static_cast<float>(controller->GetFrame(f - 1));

          frameDelta = (prevFrame - frameDelta) / (prevFrame - boundFrame);

          controller->Interpolate(out, f - 1, frameDelta, minMax);
          break;
        }
      }
    }
  }
}

uni::MotionTrack::TrackType_e LMTTrack_internal::TrackType() const {
  const auto iType = this->GetTrackType();

  switch (iType) {
  case TrackType_AbsolutePosition:
  case TrackType_LocalPosition:
    return MotionTrack::TrackType_e::Position;

  case TrackType_AbsoluteRotation:
  case TrackType_LocalRotation:
    return MotionTrack::TrackType_e::Rotation;

  default:
    return MotionTrack::TrackType_e::Scale;
  }
}

template <class C> class LMTTrackShared : public LMTTrack_internal {
public:
  typedef C value_type;

  std::unique_ptr<C, es::deleter_hybrid> data;

  LMTTrackShared() : data(new C) { useRefFrame = USE_REF_FRAME; }
  LMTTrackShared(C *_data, char *masterBuffer, bool swapEndian)
      : data(_data, false) {
    useRefFrame = USE_REF_FRAME;
    data->Fixup(masterBuffer, swapEndian);

    if (CreateController()) {
      controller->Assign(data->bufferOffset, data->bufferSize, swapEndian);
    }

    if constexpr (USE_EXTREMES) {
      useMinMax = true;
      memcpy(&minMax, data->extremes, sizeof(TrackMinMax));
    }
  }

  // SFINAE ACCESSORS
  template <class SC>
  using UseTrackExtremes_ = decltype(std::declval<SC>().extremes);

  template <class SC>
  using UseRefFrane_ = decltype(std::declval<SC>().referenceData);

  template <class SC> using UseBoneID2_ = decltype(std::declval<SC>().boneID2);

  // OVERRIDDES
  bool CreateController() override {
    controller = LMTTrackControllerPtr(LMTTrackController::CreateCodec(
        static_cast<uint32>(data->compression), C::SUBVERSION));

    return static_cast<bool>(controller);
  }

  TrackType_e GetTrackType() const noexcept override {
    return static_cast<TrackType_e>(data->trackType);
  }

  void SetTrackType(TrackType_e type) noexcept override {
    data->trackType = static_cast<TrackType_er>(type);
  }

  size_t Stride() const override { return sizeof(C); }

  size_t BoneIndex() const noexcept override {
    auto boneID = data->boneID;

    if constexpr (USE_BONEID2) {
      return boneID;
    } else {
      return boneID == 0xff ? -1 : boneID;
    }
  }

  uint32 BoneType() const noexcept override { return data->boneType; }

  void BoneID(int32 boneID) noexcept override {
    if constexpr (USE_BONEID2) {
      data->boneID = boneID;
    } else {
      if (boneID == -1) {
        data->boneID = 0xff;
      } else {
        data->boneID = boneID;
      }
    }
  }

  const Vector4 *GetRefData() const override {
    if constexpr (USE_REF_FRAME) {
      return data->referenceData;
    } else {
      return nullptr;
    }
  }

  // research this
  /*enabledFunction(noMHBone, int32) MirroredBoneID() const noexcept {
    return data->boneID2;
  }

  disabledFunction(noMHBone, int32) MirroredBoneID() const noexcept {
    return -1;
  }

  enabledFunction(noMHBone, void) MirroredBoneID(int32 boneID) noexcept {
    data->boneID2 = boneID;
  }

  disabledFunction(noMHBone, void) MirroredBoneID(int32 boneID) noexcept {}*/

  static constexpr bool USE_EXTREMES =
      es::is_detected_v<UseTrackExtremes_, LMTTrackShared>;

  static constexpr bool USE_REF_FRAME =
      es::is_detected_v<UseRefFrane_, LMTTrackShared>;

  static constexpr bool USE_BONEID2 =
      es::is_detected_v<UseBoneID2_, LMTTrackShared>;

  bool UseTrackExtremes() const override { return USE_EXTREMES; }

  void ReflectToXML(pugi::xml_node node) const override {
    ReflectorWrap<C> refl(data.get());
    ReflectorXMLUtil::Save(refl, node);
  }

  void ReflectFromXML(pugi::xml_node node) override {
    ReflectorWrap<C> refl(data.get());
    ReflectorXMLUtil::Load(refl, node);
  }

  void SaveInternal(BinWritterRef wr, LMTFixupStorage &storage) const override {
    const size_t cOff = wr.Tell();
    constexpr size_t numPointers = C::NUMPOINTERS;
    const auto pointers = C::Pointers();

    wr.Write(*data);

    for (size_t p = 0; p < numPointers; p++) {
      storage.SaveFrom(cOff + pointers[p]);
    }
  }
};

LMTTrack_internal::LMTTrack_internal() : controller(nullptr) {
  RegisterReflectedTypes<TrackV1BufferTypes, TrackV1_5BufferTypes,
                         TrackV2BufferTypes, TrackType_er>();
}

REFLECT(CLASS(TrackV0<esPointerX86>), MEMBER(compression), MEMBER(trackType),
        MEMBER(boneType), MEMBER(boneID), MEMBER(weight));

REFLECT(CLASS(TrackV0<esPointerX64>), MEMBER(compression), MEMBER(trackType),
        MEMBER(boneType), MEMBER(boneID), MEMBER(weight));

REFLECT(CLASS(TrackV1<esPointerX86, TrackV1BufferTypes>), MEMBER(compression),
        MEMBER(trackType), MEMBER(boneType), MEMBER(boneID), MEMBER(weight),
        MEMBER(referenceData));

REFLECT(CLASS(TrackV1<esPointerX64, TrackV1BufferTypes>), MEMBER(compression),
        MEMBER(trackType), MEMBER(boneType), MEMBER(boneID), MEMBER(weight),
        MEMBER(referenceData));

REFLECT(CLASS(TrackV1<esPointerX86, TrackV1_5BufferTypes>), MEMBER(compression),
        MEMBER(trackType), MEMBER(boneType), MEMBER(boneID), MEMBER(weight),
        MEMBER(referenceData));

REFLECT(CLASS(TrackV1<esPointerX64, TrackV1_5BufferTypes>), MEMBER(compression),
        MEMBER(trackType), MEMBER(boneType), MEMBER(boneID), MEMBER(weight),
        MEMBER(referenceData));

REFLECT(CLASS(TrackV2<esPointerX86>), MEMBER(compression), MEMBER(trackType),
        MEMBER(boneType), MEMBER(boneID), MEMBER(weight),
        MEMBER(referenceData));

REFLECT(CLASS(TrackV2<esPointerX64>), MEMBER(compression), MEMBER(trackType),
        MEMBER(boneType), MEMBER(boneID), MEMBER(weight),
        MEMBER(referenceData));

REFLECT(CLASS(TrackV3<esPointerX86>), MEMBER(compression), MEMBER(trackType),
        MEMBER(boneType), MEMBER(boneID), MEMBER(boneID2), MEMBER(weight),
        MEMBER(referenceData));

REFLECT(CLASS(TrackV3<esPointerX64>), MEMBER(compression), MEMBER(trackType),
        MEMBER(boneType), MEMBER(boneID), MEMBER(boneID2), MEMBER(weight),
        MEMBER(referenceData));

using ptr_type_ = std::unique_ptr<LMTTrack>;

template <class C> struct f_ {
  static ptr_type_ creatorBase() {
    return std::make_unique<LMTTrackShared<C>>();
  }

  static ptr_type_ creator(void *ptr, char *buff, bool endi) {
    return std::make_unique<LMTTrackShared<C>>(static_cast<C *>(ptr), buff,
                                               endi);
  }
};

static const std::map<LMTConstructorPropertiesBase,
                      decltype(&f_<void>::creatorBase)>
    trackRegistry = {
        // clang-format off
        {{LMTArchType::X64, LMTVersion::V_22}, f_<TrackV0<esPointerX64>>::creatorBase},
        {{LMTArchType::X86, LMTVersion::V_22}, f_<TrackV0<esPointerX86>>::creatorBase},
        {{LMTArchType::X64, LMTVersion::V_40}, f_<TrackV1<esPointerX64, TrackV1BufferTypes>>::creatorBase},
        {{LMTArchType::X86, LMTVersion::V_40}, f_<TrackV1<esPointerX86, TrackV1BufferTypes>>::creatorBase},
        {{LMTArchType::X64, LMTVersion::V_51}, f_<TrackV1<esPointerX64, TrackV1_5BufferTypes>>::creatorBase},
        {{LMTArchType::X86, LMTVersion::V_51}, f_<TrackV1<esPointerX86, TrackV1_5BufferTypes>>::creatorBase},
        {{LMTArchType::X64, LMTVersion::V_56}, f_<TrackV2<esPointerX64>>::creatorBase},
        {{LMTArchType::X86, LMTVersion::V_56}, f_<TrackV2<esPointerX86>>::creatorBase},
        {{LMTArchType::X64, LMTVersion::V_92}, f_<TrackV3<esPointerX64>>::creatorBase},
        {{LMTArchType::X86, LMTVersion::V_92}, f_<TrackV3<esPointerX86>>::creatorBase},
        // clang-format on
};

static const std::map<LMTConstructorPropertiesBase,
                      decltype(&f_<void>::creator)>
    trackRegistryLink = {
        // clang-format off
        {{LMTArchType::X64, LMTVersion::V_22}, f_<TrackV0<esPointerX64>>::creator},
        {{LMTArchType::X86, LMTVersion::V_22}, f_<TrackV0<esPointerX86>>::creator},
        {{LMTArchType::X64, LMTVersion::V_40}, f_<TrackV1<esPointerX64, TrackV1BufferTypes>>::creator},
        {{LMTArchType::X86, LMTVersion::V_40}, f_<TrackV1<esPointerX86, TrackV1BufferTypes>>::creator},
        {{LMTArchType::X64, LMTVersion::V_51}, f_<TrackV1<esPointerX64, TrackV1_5BufferTypes>>::creator},
        {{LMTArchType::X86, LMTVersion::V_51}, f_<TrackV1<esPointerX86, TrackV1_5BufferTypes>>::creator},
        {{LMTArchType::X64, LMTVersion::V_56}, f_<TrackV2<esPointerX64>>::creator},
        {{LMTArchType::X86, LMTVersion::V_56}, f_<TrackV2<esPointerX86>>::creator},
        {{LMTArchType::X64, LMTVersion::V_92}, f_<TrackV3<esPointerX64>>::creator},
        {{LMTArchType::X86, LMTVersion::V_92}, f_<TrackV3<esPointerX86>>::creator},
        // clang-format on
};

ptr_type_ LMTTrack::Create(const LMTConstructorProperties &props) {
  if (props.dataStart) {
    return trackRegistryLink.at(props)(props.dataStart, props.masterBuffer,
                                       props.swappedEndian);
  } else {
    return trackRegistry.at(props)();
  }
}
