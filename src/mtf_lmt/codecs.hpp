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

#include "datas/allocator_hybrid.hpp"
#include "datas/flags.hpp"

struct Buf_SingleVector3 {
  DECLARE_REFLECTOR;
  Vector data;

  static const uint32 NEWLINEMOD = 1;
  static const bool VARIABLE_SIZE = false;

  uint32 Size() const;

  void GetFrame(int32 &currentFrame) const;

  int32 GetFrame() const;

  void AppendToString(std::stringstream &buffer) const;

  void RetreiveFromString(const std::string &buffer, uint32 &bufferIter);

  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void SetFrame(uint64) {}

  void Interpolate(Vector4A16 &out, const Buf_SingleVector3 &rightFrame,
                  float delta, const TrackMinMax *) const;

  void SwapEndian();
};

struct Buf_StepRotationQuat3 : Buf_SingleVector3 {
  void Evaluate(Vector4A16 &out) const;
  void Interpolate(Vector4A16 &out, const Buf_StepRotationQuat3 &rightFrame,
                  float delta, const TrackMinMax *) const;
};

struct Buf_LinearVector3 {
  DECLARE_REFLECTOR;
  Vector data;
  uint32 additiveFrames;

  static const uint32 NEWLINEMOD = 1;
  static const bool VARIABLE_SIZE = false;

  uint32 Size() const;

  void AppendToString(std::stringstream &buffer) const;

  void RetreiveFromString(const std::string &buffer, uint32 &bufferIter);

  void Devaluate(const Vector4A16 &in);

  void Evaluate(Vector4A16 &out) const;

  void GetFrame(int32 &currentFrame) const;

  int32 GetFrame() const;

  void SetFrame(uint64 frame);

  void Interpolate(Vector4A16 &out, const Buf_LinearVector3 &rightFrame,
                  float delta, const TrackMinMax *) const;

  void SwapEndian();
};

REFLECTOR_CREATE(Buf_HermiteVector3_Flags, ENUM, 1, CLASS, InTangentX,
                 InTangentY, InTangentZ, OutTangentX, OutTangentY, OutTangentZ)

struct Buf_HermiteVector3 {
  DECLARE_REFLECTOR;

  uint8 size;
  esFlags<uint8, Buf_HermiteVector3_Flags> flags;
  uint16 additiveFrames;
  Vector data;
  float tangents[6];

  static const uint32 NEWLINEMOD = 1;
  static const bool VARIABLE_SIZE = true;

  uint32 Size() const;

  void AppendToString(std::stringstream &buffer) const;

  void RetreiveFromString(const std::string &buffer, uint32 &bufferIter);

  void Evaluate(Vector4A16 &out) const;

  void GetTangents(Vector4A16 &inTangs, Vector4A16 &outTangs) const;

  void Devaluate(const Vector4A16 &) {
    // Not Supported
  }

  void GetFrame(int32 &currentFrame) const;

  int32 GetFrame() const;

  void SetFrame(uint64 frame);

  void Interpolate(Vector4A16 &out, const Buf_HermiteVector3 &rightFrame,
                  float delta, const TrackMinMax *) const;

  void SwapEndian();
};

struct Buf_SphericalRotation {
  uint64 data;

  static const uint32 NEWLINEMOD = 4;
  static const bool VARIABLE_SIZE = false;
  static const uint32 MAXFRAMES = 255;

  uint32 Size() const;

  void AppendToString(std::stringstream &buffer) const;

  void RetreiveFromString(const std::string &buffer, uint32 &bufferIter);

  static const uint64 componentMask = (1 << 17) - 1;
  static const uint64 componentMaskW = (1 << 19) - 1;
  static const float componentMultiplier;
  static const float componentMultiplierInv;
  static const float componentMultiplierW;
  static const uint64 dataField = (1ULL << 56) - 1;
  static const uint64 frameField = ~dataField;

  void Interpolate(Vector4A16 &out, const Buf_SphericalRotation &rightFrame,
                  float delta, const TrackMinMax *minMax) const;

  void Devaluate(const Vector4A16 &in);

  void Evaluate(Vector4A16 &out) const;

  void GetFrame(int32 &currentFrame) const;

  int32 GetFrame() const;

  void SetFrame(uint64 frame);

  void SwapEndian();
};

struct Buf_BiLinearVector3_16bit {
  USVector data;
  uint16 additiveFrames;

  static const uint32 NEWLINEMOD = 4;
  static const bool VARIABLE_SIZE = false;

  uint32 Size() const;

  void AppendToString(std::stringstream &buffer) const;

  void RetreiveFromString(const std::string &buffer, uint32 &bufferIter);

  static const uint64 componentMask = 0xffff;
  static const Vector4A16 componentMultiplier;
  static const Vector4A16 componentMultiplierInv;

  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void GetFrame(int32 &currentFrame) const;

  int32 GetFrame() const;

  void SetFrame(uint64 frame);

  void Interpolate(Vector4A16 &out, const Buf_BiLinearVector3_16bit &rightFrame,
                  float delta, const TrackMinMax *minMax) const;

  void SwapEndian();
};

struct Buf_BiLinearVector3_8bit {
  UCVector data;
  uint8 additiveFrames;

  static const uint32 NEWLINEMOD = 7;
  static const bool VARIABLE_SIZE = false;

  uint32 Size() const;

  void AppendToString(std::stringstream &buffer) const;

  void RetreiveFromString(const std::string &buffer, uint32 &bufferIter);

  static const uint64 componentMask = 0xff;
  static const Vector4A16 componentMultiplier;
  static const Vector4A16 componentMultiplierInv;

  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void GetFrame(int32 &currentFrame) const;

  int32 GetFrame() const;

  void SetFrame(uint64 frame);

  void Interpolate(Vector4A16 &out, const Buf_BiLinearVector3_8bit &rightFrame,
                  float delta, const TrackMinMax *minMax) const;

  void SwapEndian() {}
};

struct Buf_LinearRotationQuat4_14bit : Buf_SphericalRotation {
  static const uint64 componentMask = (1 << 14) - 1;
  static const float componentMultiplier;
  static const float componentSignMax;
  static const float componentMultiplierInv;

  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void Interpolate(Vector4A16 &out,
                  const Buf_LinearRotationQuat4_14bit &rightFrame, float delta,
                  const TrackMinMax *minMax) const;
};

struct Buf_BiLinearRotationQuat4_7bit {
  uint32 data;

  static const uint32 NEWLINEMOD = 8;
  static const bool VARIABLE_SIZE = false;

  uint32 Size() const;

  void AppendToString(std::stringstream &buffer) const;

  void RetreiveFromString(const std::string &buffer, uint32 &bufferIter);

  static const uint32 componentMask = (1 << 7) - 1;
  static const float componentMultiplier;
  static const float componentMultiplierInv;
  static const uint32 dataField = (1 << 28) - 1;
  static const uint32 frameField = ~dataField;

  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void GetFrame(int32 &currentFrame) const;

  int32 GetFrame() const;

  void SetFrame(uint64 frame);

  void Interpolate(Vector4A16 &out,
                  const Buf_BiLinearRotationQuat4_7bit &rightFrame, float delta,
                  const TrackMinMax *minMax) const;

  void SwapEndian();
};

struct Buf_BiLinearRotationQuatXW_14bit : Buf_BiLinearRotationQuat4_7bit {
  static const uint32 componentMask = (1 << 14) - 1;
  static const float componentMultiplier;
  static const float componentMultiplierInv;

  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void Interpolate(Vector4A16 &out,
                  const Buf_BiLinearRotationQuatXW_14bit &rightFrame,
                  float delta, const TrackMinMax *minMax) const;
};

struct Buf_BiLinearRotationQuatYW_14bit : Buf_BiLinearRotationQuatXW_14bit {
  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void Interpolate(Vector4A16 &out,
                  const Buf_BiLinearRotationQuatYW_14bit &rightFrame,
                  float delta, const TrackMinMax *minMax) const;
};

struct Buf_BiLinearRotationQuatZW_14bit : Buf_BiLinearRotationQuatXW_14bit {
  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void Interpolate(Vector4A16 &out,
                  const Buf_BiLinearRotationQuatZW_14bit &rightFrame,
                  float delta, const TrackMinMax *minMax) const;
};

struct Buf_BiLinearRotationQuat4_11bit {
  USVector data;

  static const uint32 NEWLINEMOD = 6;
  static const bool VARIABLE_SIZE = false;

  uint32 Size() const;

  void AppendToString(std::stringstream &buffer) const;

  void RetreiveFromString(const std::string &buffer, uint32 &bufferIter);

  static const uint64 componentMask = (1 << 11) - 1;
  static const float componentMultiplier;
  static const float componentMultiplierInv;

  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void GetFrame(int32 &currentFrame) const;

  void Interpolate(Vector4A16 &out,
                  const Buf_BiLinearRotationQuat4_11bit &rightFrame,
                  float delta, const TrackMinMax *minMax) const;

  void SwapEndian();
};

struct Buf_BiLinearRotationQuat4_9bit {
  uint8 data[5];

  static const uint32 NEWLINEMOD = 6;
  static const bool VARIABLE_SIZE = false;

  uint32 Size() const;

  void AppendToString(std::stringstream &buffer) const;

  void RetreiveFromString(const std::string &buffer, uint32 &bufferIter);

  static const uint64 componentMask = (1 << 9) - 1;
  static const float componentMultiplier;
  static const float componentMultiplierInv;

  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void GetFrame(int32 &currentFrame) const;

  void Interpolate(Vector4A16 &out,
                  const Buf_BiLinearRotationQuat4_9bit &rightFrame, float delta,
                  const TrackMinMax *minMax) const;

  void SwapEndian() {}
};

template <class C> struct Buff_EvalShared : LMTTrackController {
  typedef std::vector<C, es::allocator_hybrid<C>> Store_Type;

  Store_Type data;
  std::vector<int16> frames;

  int32 GetFrame(uint32 frame) const override { return frames[frame]; }
  uint32 NumFrames() const override { return static_cast<uint32>(data.size()); }
  void NumFrames(uint32 numItems) override { data.resize(numItems); }
  bool IsCubic() const override { return C::VARIABLE_SIZE; }

  template <class _C = C>
  typename std::enable_if<_C::VARIABLE_SIZE>::type
  _GetTangents(Vector4A16 &inTangs, Vector4A16 &outTangs, uint32 frame) const {
    data[frame].GetTangents(inTangs, outTangs);
  }

  template <class _C = C>
  typename std::enable_if<!_C::VARIABLE_SIZE>::type
  _GetTangents(Vector4A16 &inTangs, Vector4A16 &outTangs, uint32 frame) const {}

  void GetTangents(Vector4A16 &inTangs, Vector4A16 &outTangs,
                   uint32 frame) const override {
    _GetTangents(inTangs, outTangs, frame);
  }

  void Evaluate(Vector4A16 &out, uint32 frame) const override {
    data[frame].Evaluate(out);
  }

  void Interpolate(Vector4A16 &out, uint32 frame, float delta,
                   TrackMinMax *bounds) const override {
    data[frame].Interpolate(out, data[frame + 1], delta, bounds);
  }

  void Devaluate(const Vector4A16 &in, uint32 frame) override {
    data[frame].Devaluate(in);
  }

  void ToString(std::string &strBuff, uint32 numIdents) const override;

  void FromString(std::string &input) override;

  void Assign(char *ptr, uint32 size) override;

  void SwapEndian() override;

  void Save(BinWritterRef wr) const override;
};
