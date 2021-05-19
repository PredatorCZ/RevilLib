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
#include "motion_78.hpp"

struct REMotion458 : public REAssetBase {
  uint64 pad;
  esPointerX64<REArray<REMotionBone>> bones;
  esPointerX64<REMotionTrack78> tracks;
  esPointerX64<char> unkOffset[7];
  esPointerX64<char16_t> animationName;
  float intervals[4];
  uint16 numBones;
  uint16 numTracks;
  uint8 numClips;
  uint8 numUnk[3];
  uint32 framesPerSecond;
  uint16 unks00[2];

  void Fixup();
};

class REMotion458Asset final : public REMotion43Asset {
public:
  REMotion458 &Get() { return REAssetBase::Get<REMotion458>(this->buffer); }
  const REMotion458 &Get() const {
    return REAssetBase::Get<const REMotion458>(this->buffer);
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
  static constexpr uint64 VERSION = 458;
};
