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
#include "datas/flags.hpp"
#include "datas/reflector.hpp"
#include "internal.hpp"

static constexpr float fPI = 3.14159265f;
static constexpr float fPI2 = 0.5 * fPI;

struct Buf_SingleVector3 {
  Vector data;

  static constexpr size_t NEWLINEMOD = 1;
  static constexpr bool VARIABLE_SIZE = false;

  size_t Size() const;

  void GetFrame(int32 &currentFrame) const;

  int32 GetFrame() const;

  void AppendToString(std::stringstream &buffer) const;

  std::string_view RetreiveFromString(std::string_view buffer);

  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void SetFrame(uint64) {}

  void Interpolate(Vector4A16 &out, const Buf_SingleVector3 &rightFrame,
                   float delta, const TrackMinMax &) const;

  void SwapEndian();
};

struct Buf_StepRotationQuat3 : Buf_SingleVector3 {
  void Evaluate(Vector4A16 &out) const;
  void Interpolate(Vector4A16 &out, const Buf_StepRotationQuat3 &rightFrame,
                   float delta, const TrackMinMax &) const;
};

struct Buf_LinearVector3 {
  Vector data;
  uint32 additiveFrames;

  static constexpr size_t NEWLINEMOD = 1;
  static constexpr bool VARIABLE_SIZE = false;

  size_t Size() const;

  void AppendToString(std::stringstream &buffer) const;

  std::string_view RetreiveFromString(std::string_view buffer);

  void Devaluate(const Vector4A16 &in);

  void Evaluate(Vector4A16 &out) const;

  void GetFrame(int32 &currentFrame) const;

  int32 GetFrame() const;

  void SetFrame(uint64 frame);

  void Interpolate(Vector4A16 &out, const Buf_LinearVector3 &rightFrame,
                   float delta, const TrackMinMax &) const;

  void SwapEndian();
};

MAKE_ENUM(ENUMSCOPE(class Buf_HermiteVector3_Flags
                    : uint8, Buf_HermiteVector3_Flags),
          EMEMBER(InTangentX), EMEMBER(InTangentY), EMEMBER(InTangentZ),
          EMEMBER(OutTangentX), EMEMBER(OutTangentY), EMEMBER(OutTangentZ))

struct Buf_HermiteVector3 {
  uint8 size;
  es::Flags<Buf_HermiteVector3_Flags> flags;
  uint16 additiveFrames;
  Vector data;
  float tangents[6];

  static constexpr size_t NEWLINEMOD = 1;
  static constexpr bool VARIABLE_SIZE = true;

  size_t Size() const;

  void AppendToString(std::stringstream &buffer) const;

  std::string_view RetreiveFromString(std::string_view buffer);

  void Evaluate(Vector4A16 &out) const;

  void GetTangents(Vector4A16 &inTangs, Vector4A16 &outTangs) const;

  void Devaluate(const Vector4A16 &) {
    // Not Supported
  }

  void GetFrame(int32 &currentFrame) const;

  int32 GetFrame() const;

  void SetFrame(uint64 frame);

  void Interpolate(Vector4A16 &out, const Buf_HermiteVector3 &rightFrame,
                   float delta, const TrackMinMax &) const;

  void SwapEndian();
};

struct Buf_SphericalRotation {
  uint64 data;

  static constexpr size_t NEWLINEMOD = 4;
  static constexpr bool VARIABLE_SIZE = false;
  static constexpr size_t MAXFRAMES = 255;

  size_t Size() const;

  void AppendToString(std::stringstream &buffer) const;

  std::string_view RetreiveFromString(std::string_view buffer);

  static constexpr uint64 componentMask = (1 << 17) - 1;
  static constexpr uint64 componentMaskW = (1 << 19) - 1;
  static constexpr float componentMultiplierInv =
      static_cast<float>(componentMask) / fPI2;
  static constexpr float componentMultiplier = 1.0f / componentMultiplierInv;
  static constexpr float componentMultiplierW =
      1.0f / static_cast<float>(componentMaskW);
  static constexpr uint64 dataField = (1ULL << 56) - 1;
  static constexpr uint64 frameField = ~dataField;

  void Interpolate(Vector4A16 &out, const Buf_SphericalRotation &rightFrame,
                   float delta, const TrackMinMax &) const;

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

  static constexpr size_t NEWLINEMOD = 4;
  static constexpr bool VARIABLE_SIZE = false;

  size_t Size() const;

  void AppendToString(std::stringstream &buffer) const;

  std::string_view RetreiveFromString(std::string_view buffer);

  static constexpr uint64 componentMask = 0xffff;
  static const Vector4A16 componentMultiplier;
  static const Vector4A16 componentMultiplierInv;

  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void GetFrame(int32 &currentFrame) const;

  int32 GetFrame() const;

  void SetFrame(uint64 frame);

  void Interpolate(Vector4A16 &out, const Buf_BiLinearVector3_16bit &rightFrame,
                   float delta, const TrackMinMax &minMax) const;

  void SwapEndian();
};

struct Buf_BiLinearVector3_8bit {
  UCVector data;
  uint8 additiveFrames;

  static constexpr size_t NEWLINEMOD = 7;
  static constexpr bool VARIABLE_SIZE = false;

  size_t Size() const;

  void AppendToString(std::stringstream &buffer) const;

  std::string_view RetreiveFromString(std::string_view buffer);

  static constexpr uint64 componentMask = 0xff;
  static const Vector4A16 componentMultiplier;
  static const Vector4A16 componentMultiplierInv;

  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void GetFrame(int32 &currentFrame) const;

  int32 GetFrame() const;

  void SetFrame(uint64 frame);

  void Interpolate(Vector4A16 &out, const Buf_BiLinearVector3_8bit &rightFrame,
                   float delta, const TrackMinMax &minMax) const;

  void SwapEndian() {}
};

struct Buf_LinearRotationQuat4_14bit : Buf_SphericalRotation {
  static constexpr uint64 componentMask = (1 << 14) - 1;
  static constexpr float componentSignMax =
      static_cast<float>(componentMask) * 0.5f;
  static constexpr float componentMultiplierInv =
      static_cast<float>(componentMask) / 4.0f;
  static constexpr float componentMultiplier = 1.0f / componentMultiplierInv;

  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void Interpolate(Vector4A16 &out,
                   const Buf_LinearRotationQuat4_14bit &rightFrame, float delta,
                   const TrackMinMax &minMax) const;
};

struct Buf_BiLinearRotationQuat4_7bit {
  uint32 data;

  static constexpr size_t NEWLINEMOD = 8;
  static constexpr bool VARIABLE_SIZE = false;

  size_t Size() const;

  void AppendToString(std::stringstream &buffer) const;

  std::string_view RetreiveFromString(std::string_view buffer);

  static constexpr uint32 componentMask = (1 << 7) - 1;
  static constexpr float componentMultiplierInv =
      static_cast<float>(componentMask);
  static constexpr float componentMultiplier = 1.0f / componentMultiplierInv;
  static constexpr uint32 dataField = (1 << 28) - 1;
  static constexpr uint32 frameField = ~dataField;

  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void GetFrame(int32 &currentFrame) const;

  int32 GetFrame() const;

  void SetFrame(uint64 frame);

  void Interpolate(Vector4A16 &out,
                   const Buf_BiLinearRotationQuat4_7bit &rightFrame,
                   float delta, const TrackMinMax &minMax) const;

  void SwapEndian();
};

struct Buf_BiLinearRotationQuatXW_14bit : Buf_BiLinearRotationQuat4_7bit {
  static constexpr uint32 componentMask = (1 << 14) - 1;
  static constexpr float componentMultiplierInv =
      static_cast<float>(componentMask);
  static constexpr float componentMultiplier = 1.0f / componentMultiplierInv;

  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void Interpolate(Vector4A16 &out,
                   const Buf_BiLinearRotationQuatXW_14bit &rightFrame,
                   float delta, const TrackMinMax &minMax) const;
};

struct Buf_BiLinearRotationQuatYW_14bit : Buf_BiLinearRotationQuatXW_14bit {
  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void Interpolate(Vector4A16 &out,
                   const Buf_BiLinearRotationQuatYW_14bit &rightFrame,
                   float delta, const TrackMinMax &minMax) const;
};

struct Buf_BiLinearRotationQuatZW_14bit : Buf_BiLinearRotationQuatXW_14bit {
  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void Interpolate(Vector4A16 &out,
                   const Buf_BiLinearRotationQuatZW_14bit &rightFrame,
                   float delta, const TrackMinMax &minMax) const;
};

struct Buf_BiLinearRotationQuat4_11bit {
  USVector data;

  static constexpr size_t NEWLINEMOD = 6;
  static constexpr bool VARIABLE_SIZE = false;

  size_t Size() const;

  void AppendToString(std::stringstream &buffer) const;

  std::string_view RetreiveFromString(std::string_view buffer);

  static constexpr uint64 componentMask = (1 << 11) - 1;
  static constexpr float componentMultiplierInv =
      static_cast<float>(componentMask);
  static constexpr float componentMultiplier = 1.0f / componentMultiplierInv;

  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void GetFrame(int32 &currentFrame) const;

  void Interpolate(Vector4A16 &out,
                   const Buf_BiLinearRotationQuat4_11bit &rightFrame,
                   float delta, const TrackMinMax &minMax) const;

  void SwapEndian();
};

struct Buf_BiLinearRotationQuat4_9bit {
  uint8 data[5];

  static constexpr size_t NEWLINEMOD = 6;
  static constexpr bool VARIABLE_SIZE = false;

  size_t Size() const;

  void AppendToString(std::stringstream &buffer) const;

  std::string_view RetreiveFromString(std::string_view buffer);

  static constexpr uint64 componentMask = (1 << 9) - 1;
  static constexpr float componentMultiplierInv =
      static_cast<float>(componentMask);
  static constexpr float componentMultiplier = 1.0f / componentMultiplierInv;

  void Evaluate(Vector4A16 &out) const;

  void Devaluate(const Vector4A16 &in);

  void GetFrame(int32 &currentFrame) const;

  void Interpolate(Vector4A16 &out,
                   const Buf_BiLinearRotationQuat4_9bit &rightFrame,
                   float delta, const TrackMinMax &minMax) const;

  void SwapEndian() {}
};

template <class C> struct Buff_EvalShared : LMTTrackController {
  typedef std::vector<C, es::allocator_hybrid<C>> Store_Type;

  Store_Type data;
  std::vector<int16> frames;

  int32 GetFrame(size_t frame) const override { return frames[frame]; }
  size_t NumFrames() const override { return data.size(); }
  void NumFrames(size_t numItems) override { data.resize(numItems); }
  bool IsCubic() const override { return C::VARIABLE_SIZE; }

  template <class _C = C>
  typename std::enable_if<_C::VARIABLE_SIZE>::type
  GetTangents_(Vector4A16 &inTangs, Vector4A16 &outTangs, size_t frame) const {
    data.at(frame).GetTangents(inTangs, outTangs);
  }

  template <class _C = C>
  typename std::enable_if<!_C::VARIABLE_SIZE>::type
  GetTangents_(Vector4A16 &, Vector4A16 &, size_t) const {}

  void GetTangents(Vector4A16 &inTangs, Vector4A16 &outTangs,
                   size_t frame) const override {
    GetTangents_(inTangs, outTangs, frame);
  }

  void Evaluate(Vector4A16 &out, size_t frame) const override {
    data.at(frame).Evaluate(out);
  }

  void Interpolate(Vector4A16 &out, size_t frame, float delta,
                   const TrackMinMax &bounds) const override {
    data.at(frame).Interpolate(out, data[frame + 1], delta, bounds);
  }

  void Devaluate(const Vector4A16 &in, size_t frame) override {
    data.at(frame).Devaluate(in);
  }

  void ToString(std::string &strBuff, size_t numIdents) const override;

  void FromString(std::string_view input) override;

  void Assign(char *ptr, size_t size, bool swapEndian) override;

  void SwapEndian() override;

  void Save(BinWritterRef wr) const override;
};
