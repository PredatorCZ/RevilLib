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

class LMTTrack_internal : public LMTTrack {
  virtual void ReflectFromXML(pugi::xml_node &node) = 0;
  virtual void ReflectToXML(pugi::xml_node &node) const = 0;
  virtual bool UseTrackExtremes() const = 0;
  virtual bool CreateController() = 0;
  virtual void SetTrackType(TrackType_e type) noexcept = 0;
  virtual void BoneID(int boneID) noexcept = 0;
  virtual const Vector4 *GetRefData() const = 0;

public:
  using MinMaxPtr = uni::Element<TrackMinMax>;
  using LMTTrackControllerPtr = std::unique_ptr<LMTTrackController>;

  int useRefFrame = 1;
  mutable float frameRate = 60.f;
  mutable uint32 numIdents = 5;
  MinMaxPtr minMax;
  LMTTrackControllerPtr controller;

  size_t NumFrames() const override;
  bool IsCubic() const override;
  void GetTangents(Vector4A16 &inTangs, Vector4A16 &outTangs,
                   size_t frame) const override;
  void Evaluate(Vector4A16 &out, size_t frame) const override;
  void GetValue(Vector4A16 &output, float time) const override;
  int32 GetFrame(size_t frame) const override;

  MotionTrack::TrackType_e TrackType() const override;

  void Load(pugi::xml_node &node) override;
  void Save(pugi::xml_node &node, bool standAlone) const override;

  void SaveBuffers(BinWritterRef wr, LMTFixupStorage &storage) const;
  virtual void SaveInternal(BinWritterRef wr, LMTFixupStorage &storage) const = 0;

  LMTTrack_internal();
};
