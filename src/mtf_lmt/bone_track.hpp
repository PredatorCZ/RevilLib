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
#include "internal.hpp"

struct LMTTrackInterface : LMTTrack {
  virtual bool UseTrackExtremes() const = 0;
  virtual const Vector4A16 GetRefData() const = 0;

  using LMTTrackControllerPtr = std::unique_ptr<LMTTrackController>;

  TrackMinMax minMax;
  mutable float frameRate = 60.f;
  mutable uint8 numIdents = 5;
  uint8 useRefFrame = 1;
  bool useMinMax = false;
  LMTTrackControllerPtr controller;

  size_t NumFrames() const override;
  bool IsCubic() const override;
  void GetTangents(Vector4A16 &inTangs, Vector4A16 &outTangs,
                   size_t frame) const override;
  void Evaluate(Vector4A16 &out, size_t frame) const override;
  void GetValue(Vector4A16 &output, float time) const override;
  int32 GetFrame(size_t frame) const override;

  MotionTrack::TrackType_e TrackType() const override;
};
