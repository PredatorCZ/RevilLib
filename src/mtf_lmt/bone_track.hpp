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
  virtual void _FromXML(pugi::xml_node &node) = 0;
  virtual void _ToXML(pugi::xml_node &node) const = 0;
  virtual bool UseTrackExtremes() const = 0;
  virtual bool CreateController() = 0;
  virtual void SetTrackType(TrackType_e type) noexcept = 0;
  virtual void BoneID(int boneID) noexcept = 0;
  virtual const Vector4 *GetRefData() const = 0;

public:
  typedef std::unique_ptr<TrackMinMax, es::deleter_hybrid> MinMaxPtr;
  typedef std::unique_ptr<LMTTrackController> LMTTrackControllerPtr;

  int useRefFrame = 1;
  mutable float frameRate = 60.f;
  mutable uint32 numIdents = 5;
  MinMaxPtr minMax;
  LMTTrackControllerPtr controller;

  uint32 NumFrames() const override;
  bool IsCubic() const override;
  void GetTangents(Vector4A16 &inTangs, Vector4A16 &outTangs,
                   uint32 frame) const override;
  void Evaluate(Vector4A16 &out, uint32 frame) const override;
  void GetValue(Vector4A16 &output, float time) const override;
  int32 GetFrame(uint32 frame) const override;

  int FromXML(pugi::xml_node &node) override;
  int ToXML(pugi::xml_node &node, bool standAlone) const override;

  int SaveBuffers(BinWritterRef wr, LMTFixupStorage &storage) const;
  virtual void Save(BinWritterRef wr, LMTFixupStorage &storage) const = 0;

  void SwapEndian();

  LMTTrack_internal();
};
