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

#pragma once
#include "asset.hpp"
#include "datas/flags.hpp"
#include "datas/unicode.hpp"
#include "uni/list_vector.hpp"
#include "uni/motion.hpp"

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

  void Fixup(char *masterBuffer);
};

struct REMimMaxBounds {
  Vector4 min;
  Vector4 max;
};

struct RETrackController {
  using Ptr = std::unique_ptr<RETrackController>;
  virtual void Assign(RETrackCurve43 *iCurve) = 0;
  virtual void Assign(RETrackCurve65 *iCurve) = 0;
  virtual void Assign(RETrackCurve78 *iCurve) = 0;
  virtual uint16 GetFrame(uint32 id) const = 0;
  virtual void Evaluate(uint32 id, Vector4A16 &out) const = 0;
  virtual ~RETrackController() {}
};

struct RETrackCurve43 {
  uint32 flags;
  uint32 numFrames, framesPerSecond;
  float duration;
  esPointerX64<uint8> frames;
  esPointerX64<char> controlPoints;
  esPointerX64<REMimMaxBounds> minMaxBounds;

  RETrackController::Ptr GetController();
  void Fixup(char *masterBuffer);
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

  void Fixup(char *masterBuffer);
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

  void Fixup();
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

typedef REMotion_t<REMotionTrack43> REMotion43;

class REMotion43Asset
    : public REAsset_internal,
      public uni::Motion,
      protected uni::VectorList<uni::MotionTrack, REMotionTrackWorker> {
public:
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

  operator uni::Element<const uni::Motion>() const {
    return uni::Element<const uni::Motion>{this, false};
  }

  void Fixup() override;
  void Build() override;

public:
  static constexpr uint64 ID = CompileFourCC("mot ");
  static constexpr uint64 VERSION = 43;
};
