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

#include <map>
#include <memory>

REFLECTOR_CREATE(TrackType_er, ENUM, 2, CLASS, 8, LocalRotation, LocalPosition,
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

template <template <class C> class PtrType>
struct TrackV0 {
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
    if (bufferOffset.Fixed()) {
      return;
    }

    if (swapEndian) {
      SwapEndian();
    }

    bufferOffset.Fixup(masterBuffer);
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

template <template <class C> class PtrType, class BufferType>
struct TrackV1 {
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
    if (bufferOffset.Fixed()) {
      return;
    }

    if (swapEndian) {
      SwapEndian();
    }

    bufferOffset.Fixup(masterBuffer);
  }

  static const size_t *Pointers() {
    static const size_t ptrs[]{
        offsetof(TrackV1, bufferOffset),
    };

    return ptrs;
  }
};

template <template <class C> class PtrType>
struct TrackV2 {
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
    if (bufferOffset.Fixed()) {
      return;
    }

    if (swapEndian) {
      SwapEndian();
    }

    bufferOffset.Fixup(masterBuffer);
    extremes.Fixup(masterBuffer);
  }

  static const size_t *Pointers() {
    static const size_t ptrs[]{
        offsetof(TrackV2, bufferOffset), offsetof(TrackV2, extremes),
    };

    return ptrs;
  }
};

template <template <class C> class PtrType>
struct TrackV3 {
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
    if (bufferOffset.Fixed()) {
      return;
    }

    if (swapEndian) {
      SwapEndian();
    }

    bufferOffset.Fixup(masterBuffer);
    extremes.Fixup(masterBuffer);
  }

  static const size_t *Pointers() {
    static const size_t ptrs[]{
        offsetof(TrackV3, bufferOffset), offsetof(TrackV3, extremes),
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

  LMTTrackShared() : data(new C) { useRefFrame = UseRefFrame_(0); }
  LMTTrackShared(C *_data, char *masterBuffer, bool swapEndian)
      : data(_data, false) {
    useRefFrame = UseRefFrame_(0);
    data->Fixup(masterBuffer, swapEndian);

    if (CreateController()) {
      controller->Assign(data->bufferOffset, data->bufferSize, swapEndian);
    }

    auto rExtremes = GetTrackExtremes_(0);
    useMinMax = rExtremes;

    if (useMinMax) {
      memcpy(&minMax, rExtremes, sizeof(TrackMinMax));
    }
  }

  // SFINAE ACCESSORS
  template <class SC = C>
  auto UseTrackExtremes_(int) const
      -> decltype(std::declval<SC>().extremes, true) {
    return true;
  }

  template <class SC = C> bool UseTrackExtremes_(...) const { return false; }

  template <class SC = C>
  auto GetTrackExtremes_(int)
      -> decltype(std::declval<SC>().extremes, (TrackMinMax *)nullptr) {
    return data->extremes;
  }

  template <class SC = C> TrackMinMax *GetTrackExtremes_(...) {
    return nullptr;
  }

  template <class SC = C>
  auto UseRefFrame_(int) const
      -> decltype(std::declval<SC>().referenceData, (int)0) {
    return 1;
  }

  template <class SC = C> int UseRefFrame_(...) const { return 0; }

  template <class SC = C>
  auto RefData_(int) const
      -> decltype(std::declval<SC>().referenceData, (const Vector4 *)nullptr) {
    return &data->referenceData;
  }

  template <class SC = C> const Vector4 *RefData_(...) const { return nullptr; }

  template <class SC = C>
  auto BoneID_(int) const -> decltype(std::declval<SC>().boneID2, (int32)0) {
    return data->boneID;
  }

  template <class SC = C> int32 BoneID_(...) const {
    return data->boneID == 0xff ? -1 : data->boneID;
  }

  template <class SC = C>
  auto SetBoneID_(int32 id, int)
      -> decltype(std::declval<SC>().boneID2, void()) {
    data->boneID = id;
  }

  template <class SC = C> void SetBoneID_(int32 id, ...) {
    data->boneID = id == -1 ? 0xff : id;
  }

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

  size_t BoneIndex() const noexcept override { return BoneID_(0); }

  uint32 BoneType() const noexcept override { return data->boneType; }

  void BoneID(int32 boneID) noexcept override { SetBoneID_(boneID, 0); }

  const Vector4 *GetRefData() const override { return RefData_(0); }

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

  bool UseTrackExtremes() const override { return UseTrackExtremes_(0); }

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

REFLECTOR_CREATE((TrackV0<esPointerX86>), 2, VARNAMES, TEMPLATE, compression,
                 trackType, boneType, boneID, weight);

REFLECTOR_CREATE((TrackV0<esPointerX64>), 2, VARNAMES, TEMPLATE, compression,
                 trackType, boneType, boneID, weight);

REFLECTOR_CREATE((TrackV1<esPointerX86, TrackV1BufferTypes>), 2, VARNAMES,
                 TEMPLATE, compression, trackType, boneType, boneID, weight,
                 referenceData);

REFLECTOR_CREATE((TrackV1<esPointerX64, TrackV1BufferTypes>), 2, VARNAMES,
                 TEMPLATE, compression, trackType, boneType, boneID, weight,
                 referenceData);

REFLECTOR_CREATE((TrackV1<esPointerX86, TrackV1_5BufferTypes>), 2, VARNAMES,
                 TEMPLATE, compression, trackType, boneType, boneID, weight,
                 referenceData);

REFLECTOR_CREATE((TrackV1<esPointerX64, TrackV1_5BufferTypes>), 2, VARNAMES,
                 TEMPLATE, compression, trackType, boneType, boneID, weight,
                 referenceData);

REFLECTOR_CREATE((TrackV2<esPointerX86>), 2, VARNAMES, TEMPLATE, compression,
                 trackType, boneType, boneID, weight, referenceData);

REFLECTOR_CREATE((TrackV2<esPointerX64>), 2, VARNAMES, TEMPLATE, compression,
                 trackType, boneType, boneID, weight, referenceData);

REFLECTOR_CREATE((TrackV3<esPointerX86>), 2, VARNAMES, TEMPLATE, compression,
                 trackType, boneType, boneID, boneID2, weight, referenceData);

REFLECTOR_CREATE((TrackV3<esPointerX64>), 2, VARNAMES, TEMPLATE, compression,
                 trackType, boneType, boneID, boneID2, weight, referenceData);

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
