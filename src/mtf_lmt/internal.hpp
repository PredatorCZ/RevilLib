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
#include "datas/allocator_hybrid.hpp"
#include "datas/deleter_hybrid.hpp"
#include "datas/endian.hpp"
#include "datas/reflector.hpp"

#include "lmt.hpp"
#include "datas/binwritter_stream.hpp"
#include "datas/pointer.hpp"

#include <memory>
#include <vector>

struct LMTFixupStorage;

static const char *idents[] = {
    "", "\t", "\t\t", "\t\t\t", "\t\t\t\t", "\t\t\t\t\t",
};

struct TrackMinMax : ReflectorInterface<TrackMinMax> {
  Vector4A16 min;
  Vector4A16 max;
};

enum class TrackTypesShared {
  None,
  SingleVector3,
  HermiteVector3,
  StepRotationQuat3,
  SphericalRotation,
  LinearVector3,
  BiLinearVector3_16bit,
  BiLinearVector3_8bit,
  LinearRotationQuat4_14bit,
  BiLinearRotationQuat4_7bit,
  BiLinearRotationQuatXW_14bit,
  BiLinearRotationQuatYW_14bit,
  BiLinearRotationQuatZW_14bit,
  BiLinearRotationQuat4_11bit,
  BiLinearRotationQuat4_9bit
};

struct LMTTrackController {
  virtual uint32 NumFrames() const = 0;
  virtual bool IsCubic() const = 0;
  virtual void GetTangents(Vector4A16 &inTangs, Vector4A16 &outTangs,
                           uint32 frame) const = 0;
  virtual void Evaluate(Vector4A16 &out, uint32 frame) const = 0;
  virtual void Interpolate(Vector4A16 &out, uint32 frame, float delta,
                           TrackMinMax *bounds = nullptr) const = 0;
  virtual int32 GetFrame(uint32 frameID) const = 0;
  virtual void NumFrames(uint32 numItems) = 0;
  virtual void ToString(std::string &strBuf, uint32 numIdents) const = 0;

  virtual void FromString(std::string &input) = 0;
  virtual void Assign(char *ptr, uint32 size, bool swapEndian) = 0;
  virtual void SwapEndian() = 0;
  virtual void Devaluate(const Vector4A16 &in, uint32 frame) = 0;
  virtual void Save(BinWritterRef wr) const = 0;

  virtual ~LMTTrackController() {}

  static LMTTrackController *CreateCodec(uint32 type, uint32 subVersion);
  static LMTTrackController *CreateCodec(TrackTypesShared type) {
    return CreateCodec(static_cast<uint32>(type), -1);
  }
  static float GetTrackMaxFrac(TrackTypesShared type);
};
