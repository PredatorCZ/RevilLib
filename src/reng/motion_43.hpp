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

#pragma once
#include "asset.hpp"
#include "spike/type/flags.hpp"
#include "spike/type/vectors_simd.hpp"
#include "spike/uni/list_vector.hpp"
#include "spike/uni/motion.hpp"
#include "spike/util/unicode.hpp"

struct RETrackCurve43;
struct RETrackCurve65;
struct RETrackCurve78;

struct REMotionBone {
  esPointerX64<char16_t> boneName;
  esPointerX64<char16_t *> parentBoneNamePtr, firstChildBoneNamePtr,
      lastChildBoneNamePtr;
  Vector4A16 position;
  Vector4A16 rotation;
  uint32 boneID, boneHash;
};

struct REMimMaxBounds {
  Vector4 min;
  Vector4 max;
};

struct KnotSpan {
  size_t offset;
  int32 first;
  int32 second;
};

struct RETrackController {
  using Ptr = std::unique_ptr<RETrackController>;
  virtual void Assign(RETrackCurve43 *iCurve) = 0;
  virtual void Assign(RETrackCurve65 *iCurve) = 0;
  virtual void Assign(RETrackCurve78 *iCurve) = 0;
  virtual uint16 GetFrame(uint32 id) const = 0;
  virtual KnotSpan GetSpan(int32 frame) const = 0;
  virtual void Evaluate(uint32 id, Vector4A16 &out) const = 0;
  virtual ~RETrackController() = default;
  virtual const char *CodecName() const = 0;
};

struct RETrackCurve43 {
  uint32 flags;
  uint32 numFrames, framesPerSecond;
  float duration;
  esPointerX64<uint8> frames;
  esPointerX64<char> controlPoints;
  esPointerX64<REMimMaxBounds> minMaxBounds;

  RETrackController::Ptr GetController();
};

struct REMotionTrack43 {
  enum TrackType : uint16 {
    TrackType_Position,
    TrackType_Rotation,
    TrackType_Scale,
  };

  int16 unk;
  es::Flags<TrackType> usedCurves;
  uint32 boneHash;
  esPointerX64<RETrackCurve43> curves;
};

template <class trackType> struct REMotion_t : public REAssetBase {
  uint64 pad;
  esPointerX64<REArray<REMotionBone>> bones;
  esPointerX64<trackType> tracks;
  esPointerX64<char> null[5];
  esPointerX64<char> unkOffset02;
  esPointerX64<char16_t> animationName;
  float intervals[4];
  uint16 numBones;
  uint16 numTracks;
  uint16 numUNK00;
  uint16 framesPerSecond;
  uint16 unks00[2];
};

class REMotionTrackWorker : public uni::MotionTrack {
  TrackType_e TrackType() const override { return cType; }
  void GetValue(Vector4A16 &output, float time) const override;
  size_t BoneIndex() const override { return boneHash; }

public:
  std::unique_ptr<RETrackController> controller;
  TrackType_e cType;
  uint32 boneHash;
  uint32 numFrames;

  operator uni::Element<const uni::MotionTrack>() const {
    return uni::Element<const uni::MotionTrack>{this, false};
  }
};

using REMotion43 = REMotion_t<REMotionTrack43>;

class REMotion43Asset
    : public REAssetImpl,
      public uni::Motion,
      protected uni::VectorList<uni::MotionTrack, REMotionTrackWorker> {
public:
  explicit REMotion43Asset(REAssetBase *base) { Assign(base); }
  REMotion43Asset() = default;
  REMotion43Asset(const REMotion43Asset &) = delete;
  REMotion43Asset(REMotion43Asset &&) = default;
  REMotion43 &Get() { return REAssetBase::Get<REMotion43>(this->buffer); }
  const REMotion43 &Get() const {
    return REAssetBase::Get<const REMotion43>(this->buffer);
  }

  std::string Name() const override {
    return es::ToUTF8(Get().animationName.operator->());
  }
  uint32 FrameRate() const override { return Get().framesPerSecond; }
  float Duration() const override { return Get().intervals[0] / FrameRate(); }
  uni::MotionTracksConst Tracks() const override {
    return uni::MotionTracksConst(this, false);
  }
  MotionType_e MotionType() const override { return MotionType_e::Relative; }

  uni::BaseElementConst AsMotion() const override {
    return this->operator uni::Element<const uni::Motion>();
  }

  operator uni::Element<const uni::Motion>() const {
    return uni::Element<const uni::Motion>{this, false};
  }

  void Fixup(std::vector<void *> &ptrStore) override;
  void Build() override;

public:
  static constexpr uint64 ID = CompileFourCC("mot ");
  static constexpr uint64 VERSION = 43;
};
