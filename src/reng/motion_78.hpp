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
#include "motion_65.hpp"

struct RETrackCurve78 {
  uint32 flags;
  uint32 numFrames;
  esPointerX86<uint8> frames;
  esPointerX86<char> controlPoints;
  esPointerX86<REMimMaxBounds> minMaxBounds;

  RETrackController::Ptr GetController();
  void Fixup(char *masterBuffer);
};

struct REMotionTrack78 {
  uint16 unk;
  es::Flags<REMotionTrack43::TrackType> usedCurves;
  uint32 boneHash;
  esPointerX86<RETrackCurve78> curves;

  void Fixup(char *masterBuffer);
};

typedef REMotion_t<REMotionTrack78> REMotion78;

class REMotion78Asset final : public REMotion43Asset {
public:
  explicit REMotion78Asset(REAssetBase *base) { Assign(base); }
  REMotion78Asset() = default;
  REMotion78 &Get() { return REAssetBase::Get<REMotion78>(this->buffer); }
  const REMotion78 &Get() const {
    return REAssetBase::Get<const REMotion78>(this->buffer);
  }

  std::string Name() const override {
    return es::ToUTF8(Get().animationName.operator->());
  }
  uint32 FrameRate() const override { return Get().framesPerSecond; }
  float Duration() const override { return Get().intervals[0] / FrameRate(); }

  void Fixup() override;
  void Build() override;

public:
  static constexpr uint64 ID = CompileFourCC("mot ");
  static constexpr uint64 VERSION = 78;
};
