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
#include "datas/esstring.h"
#include "datas/flags.hpp"
#include "uni_list_vector.hpp"
#include "uni_motion.hpp"

struct REMotionTrack78;
struct RETrackCurve;
struct RETrackCurve78;

struct REMotionBone {
  REPointerX64<char16_t> boneName;
  REPointerX64<char16_t *> parentBoneNamePtr, firstChildBoneNamePtr,
      lastChildBoneNamePtr;
  Vector4A16 position;
  Vector4A16 rotation;
  int boneID, boneHash;
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
  virtual ushort GetFrame(int id) const = 0;
  virtual void Evaluate(int id, Vector4A16 &out) const = 0;
  virtual ~RETrackController() {}
};

struct RETrackCurve {
  int flags;
  int numFrames, framesPerSecond;
  float duration;
  REPointerX64<uchar> frames;
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

  short unk;
  esFlags<short, TrackType> usedCurves;
  uint boneHash;
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
  short numBones;
  short numTracks;
  short numUNK00;
  short framesPerSecond;
  short unks00[2];

  int Fixup();
};

class REMotionCurveWorker : public uni::MotionCurve {
  CurveType_e CurveType() const override { return cType; }
  void GetValue(uni::PRSCurve &output, float time) const override;
  void GetValue(esMatrix44 &output, float time) const override;
  void GetValue(float &output, float time) const override;
  void GetValue(Vector4A16 &output, float time) const override;
  size_t BoneIndex() const { return boneHash; }

public:
  std::unique_ptr<RETrackController> controller;
  CurveType_e cType;
  uint boneHash;
  uint numFrames;
};

class REMotionTrackWorker
    : public uni::MotionTrack,
      uni::VectorList<uni::MotionCurve, REMotionCurveWorker> {
  const uni::MotionCurves &Curves() const override { return *this; }
  size_t Index() const override { return boneHash; }

public:
  uint boneHash;

  REMotionTrackWorker(REMotionTrack *tck);
  REMotionTrackWorker(REMotionTrack78 *tck);
};

class REMotionAsset : public REAsset_internal,
                      public uni::Motion,
                      protected uni::VectorList<uni::MotionTrack, REMotionTrackWorker> {
public:
  REMotion &Get() { return REAssetBase::Get<REMotion>(this->buffer); }
  const REMotion &Get() const {
    return REAssetBase::Get<const REMotion>(this->buffer);
  }

  std::string Name() const override { // FIX THIS!!!
    return esStringConvert<char>(
        reinterpret_cast<const wchar_t *>(Get().animationName.operator->()));
  }
  void FrameRate(uint fps) override;
  uint FrameRate() const override { return Get().framesPerSecond; }
  float Duration() const override { return Get().intervals[0] / FrameRate(); }
  const uni::MotionTracks &Tracks() const override { return *this; }
  int Fixup() override;
  void Build() override;

public:
  static const uint64 ID = CompileFourCC("mot ");
  static const uint64 VERSION = 65;
};