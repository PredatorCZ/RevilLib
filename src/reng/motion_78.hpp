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
#include "motion.hpp"

struct RETrackCurve78 {
  uint32 flags;
  uint32 numFrames;
  esPointerX86<uint8> frames;
  esPointerX86<char> controlPoints;
  esPointerX86<REMimMaxBounds> minMaxBounds;

  RETrackController *GetController();
  int Fixup(char *masterBuffer);
};

struct REMotionTrack78 {
  uint16 unk;
  esFlags<uint16, REMotionTrack::TrackType> usedCurves;
  uint32 boneHash;
  esPointerX86<RETrackCurve78> curves;

  int Fixup(char *masterBuffer);
};

struct REMotion78 : public REAssetBase {
  uint64 pad;
  esPointerX64<REArray<REMotionBone>> bones;
  esPointerX64<REMotionTrack78> tracks;
  esPointerX64<char> null[5];
  esPointerX64<char> unkOffset02;
  esPointerX64<char16_t> animationName;
  float intervals[4];
  uint16 numBones;
  uint16 numTracks;
  uint16 numUNK00;
  uint16 framesPerSecond;
  uint16 unks00[2];

  int Fixup();
};

class REMotion78Asset final : public REMotionAsset {
public:
  REMotion78 &Get() { return REAssetBase::Get<REMotion78>(this->buffer); }
  const REMotion78 &Get() const {
    return REAssetBase::Get<const REMotion78>(this->buffer);
  }

  std::string Name() const override {
    return es::ToUTF8(Get().animationName.operator->());
  }
  uint32 FrameRate() const override { return Get().framesPerSecond; }
  float Duration() const override { return Get().intervals[0] / FrameRate(); }

  int Fixup() override;
  void Build() override;

public:
  static const uint64 ID = CompileFourCC("mot ");
  static const uint64 VERSION = 78;
};