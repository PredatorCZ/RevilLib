/*      Revil Format Library
        Copyright(C) 2017-2019 Lukas Cone

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
#include "LMTInternal.h"

class LMTTrack_internal : public LMTTrack {
  virtual void _FromXML(pugi::xml_node &node) = 0;
  virtual void _ToXML(pugi::xml_node &node) const = 0;
  virtual bool UseTrackExtremes() const = 0;
  virtual bool CreateController() = 0;
  virtual void SetTrackType(TrackType type) noexcept = 0;
  virtual void BoneID(int boneID) noexcept = 0;
  virtual const Vector4 *GetRefData() const = 0;

public:
  typedef std::unique_ptr<TrackMinMax, std::deleter_hybrid> MinMaxPtr;
  typedef std::unique_ptr<LMTTrackController> LMTTrackControllerPtr;

  int useRefFrame = 1;
  mutable int numIdents = 5;
  MinMaxPtr minMax;
  LMTTrackControllerPtr controller;

  int NumFrames() const override;
  bool IsCubic() const override;
  void GetTangents(Vector4A16 &inTangs, Vector4A16 &outTangs,
                   int frame) const override;
  void Evaluate(Vector4A16 &out, int frame) const override;
  void Interpolate(Vector4A16 &out, float time) const override;
  short GetFrame(int frame) const override;

  int FromXML(pugi::xml_node &node) override;
  int ToXML(pugi::xml_node &node, bool standAlone) const override;

  int SaveBuffers(BinWritter *wr, LMTFixupStorage &storage) const;
  virtual void Save(BinWritter *wr, LMTFixupStorage &storage) const = 0;

  void SwapEndian();

  LMTTrack_internal();
};
