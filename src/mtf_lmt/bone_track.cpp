/*  Revil Format Library
    Copyright(C) 2017-2023 Lukas Cone

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
#include "fixup_storage.hpp"
#include "pugixml.hpp"
#include "spike/reflect/reflector_xml.hpp"
#include "spike/uni/deleter_hybrid.hpp"

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

#include "bone_track.inl"

size_t LMTTrackInterface::NumFrames() const { return controller->NumFrames(); }

bool LMTTrackInterface::LMTTrackInterface::IsCubic() const {
  return controller->IsCubic();
}

void LMTTrackInterface::GetTangents(Vector4A16 &inTangs, Vector4A16 &outTangs,
                                    size_t frame) const {
  controller->GetTangents(inTangs, outTangs, frame);
}

void LMTTrackInterface::Evaluate(Vector4A16 &out, size_t frame) const {
  controller->Evaluate(out, frame);

  if (useMinMax) {
    out = minMax.max + minMax.min * out;
  }
}

int32 LMTTrackInterface::GetFrame(size_t frame) const {
  return controller->GetFrame(frame);
}

void LMTTrackInterface::GetValue(Vector4A16 &out, float time) const {
  float frameDelta = time * frameRate;
  int32 frame = static_cast<int32>(frameDelta);
  const size_t numCtrFrames = controller->NumFrames();

  if (!numCtrFrames) {
    if (useRefFrame) {
      out = GetRefData();
    }

    return;
  }

  if (useRefFrame) {
    if (loopFrame < 1) {
      if (!frame) {
        if (frameDelta < 0.0001f) {
          out = GetRefData();
        } else {
          frameDelta -= frame;
          Evaluate(out, 0);
          out = GetRefData() + (out - GetRefData()) * frameDelta;
        }

        return;
      }

      frame--;
      frameDelta -= 1.f;
    }
  }

  const int32 maxFrame = controller->GetFrame(numCtrFrames - 1);

  if (frame >= maxFrame) {
    Evaluate(out, numCtrFrames - 1);
  } else {
    for (size_t f = 1; f < numCtrFrames; f++) {
      int32 cFrame = controller->GetFrame(f);

      if (cFrame > frame) {
        const float boundFrame = static_cast<float>(cFrame);
        const float prevFrame = static_cast<float>(controller->GetFrame(f - 1));

        frameDelta = (prevFrame - frameDelta) / (prevFrame - boundFrame);

        controller->Interpolate(out, f - 1, frameDelta, minMax);
        break;
      }
    }
  }
}

uni::MotionTrack::TrackType_e LMTTrackInterface::TrackType() const {
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

static const TrackTypesShared buffRemapRegistry[][16] = {
    {
        TrackTypesShared::None,
        TrackTypesShared::SingleVector3,
        TrackTypesShared::SingleVector3,
        TrackTypesShared::None,
        TrackTypesShared::StepRotationQuat3,
        TrackTypesShared::HermiteVector3,
        TrackTypesShared::SphericalRotation,
        TrackTypesShared::None,
        TrackTypesShared::None,
        TrackTypesShared::LinearVector3,
    },
    {
        TrackTypesShared::None,
        TrackTypesShared::SingleVector3,
        TrackTypesShared::SingleVector3,
        TrackTypesShared::None,
        TrackTypesShared::StepRotationQuat3,
        TrackTypesShared::HermiteVector3,
        TrackTypesShared::LinearRotationQuat4_14bit,
        TrackTypesShared::None,
        TrackTypesShared::None,
        TrackTypesShared::LinearVector3,
    },
    {
        TrackTypesShared::None,
        TrackTypesShared::SingleVector3,
        TrackTypesShared::StepRotationQuat3,
        TrackTypesShared::LinearVector3,
        TrackTypesShared::BiLinearVector3_16bit,
        TrackTypesShared::BiLinearVector3_8bit,
        TrackTypesShared::LinearRotationQuat4_14bit,
        TrackTypesShared::BiLinearRotationQuat4_7bit,
        TrackTypesShared::None,
        TrackTypesShared::None,
        TrackTypesShared::None,
        TrackTypesShared::BiLinearRotationQuatXW_14bit,
        TrackTypesShared::BiLinearRotationQuatYW_14bit,
        TrackTypesShared::BiLinearRotationQuatZW_14bit,
        TrackTypesShared::BiLinearRotationQuat4_11bit,
        TrackTypesShared::BiLinearRotationQuat4_9bit,
    },
};

struct LMTTrackMidInterface : LMTTrackInterface {
  clgen::BoneTrack::Interface interface;

  LMTTrackMidInterface(clgen::LayoutLookup rules, char *data) : interface {
    data, rules
  } {
    useRefFrame = interface.m(clgen::BoneTrack::referenceData) >= 0;
  }

  TrackType_e GetTrackType() const noexcept override {
    return static_cast<TrackType_e>(interface.TrackType());
  }

  size_t Stride() const override { return interface.layout->totalSize; }

  size_t BoneIndex() const noexcept override {
    if (interface.m(clgen::BoneTrack::boneID2) >= 0) {
      return interface.BoneID2();
    }

    auto boneID = interface.BoneID();
    return boneID == 0xff ? -1 : boneID;
  }

  uint32 BoneType() const noexcept override { return interface.BoneType(); }

  const Vector4A16 GetRefData() const override {
    return interface.ReferenceData();
  }

  bool UseTrackExtremes() const override {
    return interface.m(clgen::BoneTrack::extremes) >= 0;
  }

  std::string_view CompressionType() const override {
    static const std::string_view COMPRESSIONS[]{
        "None",
        "SingleVector3",
        "HermiteVector3",
        "StepRotationQuat3",
        "SphericalRotation",
        "LinearVector3",
        "BiLinearVector3_16bit",
        "BiLinearVector3_8bit",
        "LinearRotationQuat4_14bit",
        "BiLinearRotationQuat4_7bit",
        "BiLinearRotationQuatXW_14bit",
        "BiLinearRotationQuatYW_14bit",
        "BiLinearRotationQuatZW_14bit",
        "BiLinearRotationQuat4_11bit",
        "BiLinearRotationQuat4_9bit",
    };
    uint32 version = 0;

    if (interface.LayoutVersion() >= LMT56) {
      version = 2;
    } else if (interface.LayoutVersion() >= LMT51) {
      version = 1;
    }

    uint8 compression = uint8(interface.Compression());

    return COMPRESSIONS[uint32(buffRemapRegistry[version][compression])];
  }
};

template <>
void ProcessClass(LMTTrackMidInterface &item, LMTConstructorProperties flags) {
  if (!item.interface.BufferPtr().Check(flags.ptrStore)) {
    if (flags.swapEndian) {
      clgen::EndianSwap(item.interface);
    }

    item.interface.BufferPtr().Fixup(flags.base, flags.ptrStore);

    if (item.interface.LayoutVersion() >= LMT56) {
      item.interface.ExtremesPtr().Fixup(flags.base, flags.ptrStore);

      if (auto extr = item.interface.Extremes(); extr) {
        if (flags.swapEndian) {
          FByteswapper(*extr);
        }
      }
    }
  }

  if (auto extr = item.interface.Extremes(); extr) {
    item.useMinMax = true;
    memcpy(&item.minMax, extr, sizeof(TrackMinMax));
  }

  uint32 version = 0;

  if (item.interface.LayoutVersion() >= LMT56) {
    version = 2;
  } else if (item.interface.LayoutVersion() >= LMT51) {
    version = 1;
  }

  uint8 compression = uint8(item.interface.Compression());

  item.controller = LMTTrackInterface::LMTTrackControllerPtr(
      LMTTrackController::CreateCodec(buffRemapRegistry[version][compression]));

  if (item.controller) {
    item.controller->Assign(item.interface.Buffer(),
                            item.interface.BufferSize(), flags.swapEndian);
  }
}

template <>
void ProcessClass(LMTTrackInterface &item, LMTConstructorProperties flags) {
  ProcessClass(static_cast<LMTTrackMidInterface &>(item), flags);
}

using ptr_type_ = std::unique_ptr<LMTTrack>;

ptr_type_ LMTTrack::Create(const LMTConstructorProperties &props) {
  return std::make_unique<LMTTrackMidInterface>(
      clgen::LayoutLookup{static_cast<uint8>(props.version),
                          props.arch == LMTArchType::X64, false},
      static_cast<char *>(props.dataStart));
}
