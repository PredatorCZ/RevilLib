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

struct REMotionTrack78;
struct RETrackCurve;
struct RETrackCurve78;

struct REMotionBone {
  REPointerX64<char16_t> boneName;
  REPointerX64<char16_t *> parentBoneNamePtr, firstChildBoneNamePtr,
      lastChildBoneNamePtr;
  Vector4A16 position;
  Vector4A16 rotation;
  uint32 boneID, boneHash;
  uint64 null;

  int Fixup(char *masterBuffer);
};

struct REMimMaxBounds {
  Vector4 min;
  Vector4 max;
};

struct RETrackController {
  virtual void Assign(RETrackCurve *iCurve) = 0;
  virtual void Assign(RETrackCurve78 *iCurve) = 0;
  virtual uint16 GetFrame(uint32 id) const = 0;
  virtual void Evaluate(uint32 id, Vector4A16 &out) const = 0;
  virtual ~RETrackController() {}
};

struct RETrackCurve {
  uint32 flags;
  uint32 numFrames, framesPerSecond;
  float duration;
  REPointerX64<uint8> frames;
  REPointerX64<char> controlPoints;
  REPointerX64<REMimMaxBounds> minMaxBounds;

  RETrackController *GetController();
  int Fixup(char *masterBuffer);
};

struct REMotionTrack {
  enum TrackType {
    TrackType_Position,
    TrackType_Rotation,
    TrackType_Scale,
  };

  int16 unk;
  esFlags<uint16, TrackType> usedCurves;
  uint32 boneHash;
  float weight;
  REPointerX64<RETrackCurve> curves;

  int Fixup(char *masterBuffer);
};

struct REMotion : public REAssetBase {
  uint64 pad;
  REPointerX64<REArray<REMotionBone>> bones;
  REPointerX64<REMotionTrack> tracks;
  REPointerX64<char> null[5];
  REPointerX64<char> unkOffset02;
  REPointerX64<char16_t> animationName;
  float intervals[4];
  uint16 numBones;
  uint16 numTracks;
  uint16 numUNK00;
  uint16 framesPerSecond;
  uint16 unks00[2];

  int Fixup();
};

class REMotionTrackWorker : public uni::MotionTrack {
  TrackType_e TrackType() const override { return cType; }
  void GetValue(uni::RTSValue &output, float time) const override;
  void GetValue(esMatrix44 &output, float time) const override;
  void GetValue(float &output, float time) const override;
  void GetValue(Vector4A16 &output, float time) const override;
  size_t BoneIndex() const { return boneHash; }

public:
  std::unique_ptr<RETrackController> controller;
  TrackType_e cType;
  uint32 boneHash;
  uint32 numFrames;
};

class REMotionAsset
    : public REAsset_internal,
      public uni::Motion,
      protected uni::VectorList<uni::MotionTrack, REMotionTrackWorker> {
public:
  REMotion &Get() { return REAssetBase::Get<REMotion>(this->buffer); }
  const REMotion &Get() const {
    return REAssetBase::Get<const REMotion>(this->buffer);
  }

  std::string Name() const override {
    return es::ToUTF8(Get().animationName.operator->());
  }
  void FrameRate(uint32 fps) override;
  uint32 FrameRate() const override { return Get().framesPerSecond; }
  float Duration() const override { return Get().intervals[0] / FrameRate(); }
  uni::MotionTracksConst Tracks() const override {
    return uni::MotionTracksConst(this, false);
  }
  MotionType_e MotionType() const override { return MotionType_e::Relative; }

  int Fixup() override;
  void Build() override;

public:
  static const uint64 ID = CompileFourCC("mot ");
  static const uint64 VERSION = 65;
};