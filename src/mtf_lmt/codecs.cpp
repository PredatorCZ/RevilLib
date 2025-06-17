/*  Revil Format Library
    Copyright(C) 2017-2023 Lukas Cone

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

#include "codecs.hpp"
#include "spike/io/binwritter_stream.hpp"
#include "spike/master_printer.hpp"
#include "spike/reflect/reflector_xml.hpp"
#include "spike/util/macroLoop.hpp"

#include <cmath>
#include <sstream>
#include <unordered_map>

REFLECT(CLASS(Buf_SingleVector3), MEMBER(data));
REFLECT(CLASS(Buf_LinearVector3), MEMBER(data), MEMBER(additiveFrames));
REFLECT(CLASS(Buf_HermiteVector3), MEMBER(flags), MEMBER(additiveFrames),
        MEMBER(data));

// https://en.wikipedia.org/wiki/Slerp
static Vector4A16 slerp(const Vector4A16 &v0, const Vector4A16 &_v1, float t) {
  Vector4A16 v1 = _v1;
  float dot = v0.Dot(v1);

  // If the dot product is negative, slerp won't take
  // the shorter path. Fix by reversing one quaternion.
  if (dot < 0.0f) {
    v1 *= -1;
    dot *= -1;
  }

  static const float DOT_THRESHOLD = 0.9995f;
  if (dot > DOT_THRESHOLD) {
    // If the inputs are too close for comfort, linearly interpolate
    // and normalize the result.

    Vector4A16 result = v0 + (v1 - v0) * t;
    return result.Normalize();
  }

  const float theta00 = acos(dot);   // theta00 = angle between input vectors
  const float theta01 = theta00 * t; // theta01 = angle between v0 and result
  const float theta02 = sin(theta01);
  const float theta03 = 1.0f / sin(theta00);
  const float s0 = cos(theta01) - dot * theta02 * theta03;
  const float s1 = theta02 * theta03;

  return ((v0 * s0) + (v1 * s1)).Normalized();
}

template <typename T>
Vector4A16 lerp(const Vector4A16 &v0, const Vector4A16 &v1, T t) {
  return v0 + (v1 - v0) * t;
}

static Vector4A16 additiveLerp(const TrackMinMax &minMax,
                               const Vector4A16 &value) {
  return minMax.max + minMax.min * value;
}

static Vector4A16 blerp(const Vector4A16 &v0, const Vector4A16 &v1,
                        const TrackMinMax &minMax, float t) {
  return lerp(additiveLerp(minMax, v0), additiveLerp(minMax, v1), t);
}

static Vector4A16 bslerp(const Vector4A16 &v0, const Vector4A16 &v1,
                         const TrackMinMax &minMax, float t) {
  return slerp(additiveLerp(minMax, v0), additiveLerp(minMax, v1), t);
}

template <class C>
void AppendToStringRaw(const C *clPtr, std::stringstream &buffer) {
  const uint8 *rawData = reinterpret_cast<const uint8 *>(clPtr);
  const size_t hexSize = clPtr->Size() * 2;
  size_t cBuff = 0;

  for (size_t i = 0; i < hexSize; i++) {
    bool idk = i & 1;
    char temp = 0x30 + (idk ? rawData[cBuff++] & 0xf : rawData[cBuff] >> 4);

    if (temp > 0x39) {
      temp += 7;
    }

    buffer << temp;
  }
}

template <class C>
std::string_view RetreiveFromRawString(C *clPtr, std::string_view buffer) {
  uint8 *rawData = reinterpret_cast<uint8 *>(clPtr);
  const size_t buffSize = clPtr->Size() * 2;
  size_t cBuff = 0;

  while (!buffer.empty() && cBuff < buffSize) {
    const int cRef = buffer.at(0);

    if (cRef < '0' || cRef > 'F' || (cRef > '9' && cRef < 'A')) {
      buffer.remove_prefix(1);
      continue;
    }

    if (!(cBuff & 1)) {
      rawData[cBuff] = atohLUT[cRef] << 4;
    } else {
      rawData[cBuff++] |= atohLUT[cRef];
    }

    buffer.remove_prefix(1);
  }

  if (cBuff < buffSize) {
    throw std::runtime_error("Raw buffer is too short!");
  }

  return buffer;
}

static auto SeekTo(std::string_view buffer, const char T = '\n') {
  while (!buffer.empty()) {
    if (buffer.front() == T) {
      buffer.remove_prefix(1);
      return buffer;
    }

    buffer.remove_prefix(1);
  }

  return buffer;
}

size_t Buf_SingleVector3::Size() const { return 12; }

void Buf_SingleVector3::AppendToString(std::stringstream &buffer) const {
  ReflectorWrap<const Buf_SingleVector3> tRefl(this);

  buffer << tRefl.GetReflectedValue(0);
}

std::string_view
Buf_SingleVector3::RetreiveFromString(std::string_view buffer) {
  buffer = es::SkipStartWhitespace(buffer, true);

  ReflectorWrap<Buf_SingleVector3> tRefl(this);
  tRefl.SetReflectedValue(0, buffer);
  return SeekTo(buffer);
}

void Buf_SingleVector3::Evaluate(Vector4A16 &out) const {
  out = Vector4A16(data, 1.0f);
}

void Buf_SingleVector3::Devaluate(const Vector4A16 &in) { data = in; }

void Buf_SingleVector3::GetFrame(int32 &currentFrame) const { currentFrame++; }

int32 Buf_SingleVector3::GetFrame() const { return 1; }

void Buf_SingleVector3::Interpolate(Vector4A16 &out,
                                    const Buf_SingleVector3 &rightFrame,
                                    float delta, const TrackMinMax &) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = lerp(startPoint, endPoint, delta);
}

void Buf_SingleVector3::SwapEndian() { FByteswapper(data); }

void Buf_StepRotationQuat3::Evaluate(Vector4A16 &out) const {
  out = Vector4A16(data.X, data.Y, data.Z, 0.0f);
  out.QComputeElement();
}

void Buf_StepRotationQuat3::Interpolate(Vector4A16 &out,
                                        const Buf_StepRotationQuat3 &rightFrame,
                                        float delta,
                                        const TrackMinMax &) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = slerp(startPoint, endPoint, delta);
}

size_t Buf_LinearVector3::Size() const { return 16; }

void Buf_LinearVector3::AppendToString(std::stringstream &buffer) const {
  ReflectorWrap<const Buf_LinearVector3> tRefl(this);

  buffer << "{ " << tRefl.GetReflectedValue(0) << ", "
         << tRefl.GetReflectedValue(1) << " }";
}

std::string_view
Buf_LinearVector3::RetreiveFromString(std::string_view buffer) {
  buffer = SeekTo(buffer, '{');
  buffer = es::SkipStartWhitespace(buffer, true);

  ReflectorWrap<Buf_LinearVector3> tRefl(this);
  tRefl.SetReflectedValue(0, buffer);

  buffer = SeekTo(buffer, ']');
  buffer = SeekTo(buffer, ',');
  buffer = es::SkipStartWhitespace(buffer, true);

  tRefl.SetReflectedValue(1, buffer);

  return SeekTo(buffer);
}

void Buf_LinearVector3::Devaluate(const Vector4A16 &in) { data = in; }

void Buf_LinearVector3::Evaluate(Vector4A16 &out) const {
  out = Vector4A16(data, 1.0f);
}

void Buf_LinearVector3::GetFrame(int32 &currentFrame) const {
  currentFrame += additiveFrames;
}

int32 Buf_LinearVector3::GetFrame() const { return additiveFrames; }

void Buf_LinearVector3::SetFrame(uint64 frame) {
  additiveFrames = static_cast<int32>(frame);
}

void Buf_LinearVector3::Interpolate(Vector4A16 &out,
                                    const Buf_LinearVector3 &rightFrame,
                                    float delta, const TrackMinMax &) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = lerp(startPoint, endPoint, delta);
}

void Buf_LinearVector3::SwapEndian() {
  FByteswapper(data);
  FByteswapper(additiveFrames);
}

size_t Buf_HermiteVector3::Size() const { return size; }

void Buf_HermiteVector3::AppendToString(std::stringstream &buffer) const {
  ReflectorWrap<const Buf_HermiteVector3> tRefl(this);

  buffer << "{ " << tRefl.GetReflectedValue(2) << ", "
         << tRefl.GetReflectedValue(1) << ", " << tRefl.GetReflectedValue(0);

  size_t curTang = 0;

  for (size_t f = 0; f < 6; f++) {
    if (flags[static_cast<Buf_HermiteVector3_Flags>(f)]) {
      buffer << ", " << tangents[curTang++];
    }
  }

  buffer << " }";
}

std::string_view
Buf_HermiteVector3::RetreiveFromString(std::string_view buffer) {
  buffer = SeekTo(buffer, '{');
  buffer = es::SkipStartWhitespace(buffer, true);

  ReflectorWrap<Buf_HermiteVector3> tRefl(this);
  tRefl.SetReflectedValue(2, buffer);

  buffer = SeekTo(buffer, ',');
  buffer = es::SkipStartWhitespace(buffer, true);

  tRefl.SetReflectedValue(1, buffer);

  buffer = SeekTo(buffer, ',');
  buffer = es::SkipStartWhitespace(buffer, true);

  tRefl.SetReflectedValue(0, buffer);

  buffer = SeekTo(buffer, ',');
  buffer = es::SkipStartWhitespace(buffer, true);

  size_t curTang = 0;

  for (size_t f = 0; f < 6; f++) {
    if (flags[static_cast<Buf_HermiteVector3_Flags>(f)]) {
      tangents[curTang++] = static_cast<float>(std::atof(buffer.data()));
      buffer = SeekTo(buffer, ',');
      buffer = es::SkipStartWhitespace(buffer, true);
    }
  }

  size = static_cast<uint8>(sizeof(Buf_HermiteVector3) - (6 - curTang) * 4);

  return SeekTo(buffer);
}

void Buf_HermiteVector3::Evaluate(Vector4A16 &out) const {
  out = Vector4A16(data, 1.0f);
}

void Buf_HermiteVector3::GetTangents(Vector4A16 &inTangs,
                                     Vector4A16 &outTangs) const {
  size_t currentTangIndex = 0;

  inTangs.X = flags[Buf_HermiteVector3_Flags::InTangentX]
                  ? tangents[currentTangIndex++]
                  : 0.0f;
  inTangs.Y = flags[Buf_HermiteVector3_Flags::InTangentY]
                  ? tangents[currentTangIndex++]
                  : 0.0f;
  inTangs.Z = flags[Buf_HermiteVector3_Flags::InTangentZ]
                  ? tangents[currentTangIndex++]
                  : 0.0f;
  outTangs.X = flags[Buf_HermiteVector3_Flags::OutTangentX]
                   ? tangents[currentTangIndex++]
                   : 0.0f;
  outTangs.Y = flags[Buf_HermiteVector3_Flags::OutTangentY]
                   ? tangents[currentTangIndex++]
                   : 0.0f;
  outTangs.Z = flags[Buf_HermiteVector3_Flags::OutTangentZ]
                   ? tangents[currentTangIndex++]
                   : 0.0f;
}

void Buf_HermiteVector3::GetFrame(int32 &currentFrame) const {
  currentFrame += additiveFrames;
}

int32 Buf_HermiteVector3::GetFrame() const { return additiveFrames; }

void Buf_HermiteVector3::SetFrame(uint64 frame) {
  additiveFrames = static_cast<uint16>(frame);
}

void Buf_HermiteVector3::Interpolate(Vector4A16 &out,
                                     const Buf_HermiteVector3 &rightFrame,
                                     float delta, const TrackMinMax &) const {
  Vector4A16 startPoint, endPoint, startPointOutTangent, endPointInTangent,
      dummy;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);
  GetTangents(startPointOutTangent, endPointInTangent);

  const float deltaP2 = delta * delta;
  const float deltaP3 = delta * deltaP2;
  const float deltaP32 = deltaP3 * 2;
  const float deltaP23 = deltaP2 * 3;
  const float h1 = deltaP32 - deltaP23 + 1;
  const float h2 = -deltaP32 + deltaP23;
  const float h3 = deltaP3 - 2 * deltaP2 + delta;
  const float h4 = deltaP3 - deltaP2;

  out = startPoint * h1 + endPoint * h2 + startPointOutTangent * h3 +
        endPointInTangent * h4;
}

void Buf_HermiteVector3::SwapEndian() {
  FByteswapper(additiveFrames);
  FByteswapper(data);

  size_t curTang = 0;

  for (size_t f = 0; f < 6; f++) {
    if (flags[static_cast<Buf_HermiteVector3_Flags>(f)]) {
      FByteswapper(tangents[curTang]);
      curTang++;
    }
  }
}

size_t Buf_SphericalRotation::Size() const { return 8; }

void Buf_SphericalRotation::AppendToString(std::stringstream &buffer) const {
  AppendToStringRaw(this, buffer);
}

std::string_view
Buf_SphericalRotation::RetreiveFromString(std::string_view buffer) {
  return RetreiveFromRawString(this, buffer);
}

void Buf_SphericalRotation::Devaluate(const Vector4A16 &_in) {
  Vector4A16 in = _in;
  data ^= data & dataField;

  if (in.W < 0.0f) {
    in = -in;
  }

  if (in.X < 0.0f) {
    in.X *= -1;
    data |= 1ULL << 53;
  }

  if (in.Y < 0.0f) {
    in.Y *= -1;
    data |= 1ULL << 54;
  }

  if (in.Z < 0.0f) {
    in.Z *= -1;
    data |= 1ULL << 55;
  }

  const float R = sqrtf(1.0f - in.W);
  const float magnitude_safe = sqrtf(1.0f - (in.W * in.W));
  const float magnitude = magnitude_safe < 0.001f ? 1.0f : magnitude_safe;

  const float phi = asinf(in.Y / magnitude);
  const float theta = asinf(in.X / (cosf(phi) * magnitude));

  data |= static_cast<uint64>(theta * componentMultiplierInv) & componentMask;
  data |= (static_cast<uint64>(phi * componentMultiplierInv) & componentMask)
          << 17;
  data |= (static_cast<uint64>(R * componentMaskW) & componentMaskW) << 34;
}

void Buf_SphericalRotation::Evaluate(Vector4A16 &out) const {
  out.X = static_cast<float>(data & componentMask);
  out.Y = static_cast<float>((data >> 17) & componentMask);
  float wComp =
      static_cast<float>((data >> 34) & componentMaskW) * componentMultiplierW;
  out *= componentMultiplier;

  const Vector4A16 var1(sinf(out.X), sinf(out.Y), cosf(out.X), cosf(out.Y));

  // Optimized Taylor series expansion of sin function, this is faster by 4ms
  // on debug, 1-2ms on release /wo inline Since this library is not frame
  // time heavy, it's disabled If enabled: invert out.X before sign check
  /*
  out.Z = out.X - fPI2;
  out.W = out.Y - fPI2;

  const Vector4A16 fvar1 = out * out;
  const Vector4A16 fvar2 = out * 9.53992f;
  const Vector4A16 var1 = out * (out - fPI) * (out + fPI) * (fvar1 - fvar2
  + 25.0493f) * (fvar1 + fvar2 + 25.0493f) * -0.000161476f;
  */

  wComp = 1.0f - (wComp * wComp);
  float magnitude = sqrtf(1.0f - (wComp * wComp));

  out.X = var1.X * var1.W * magnitude;
  out.Y = var1.Y * magnitude;
  out.Z = var1.Z * var1.W * magnitude;
  out.W = wComp;

  if ((data >> 53) & 1) {
    out.X *= -1;
  }

  if ((data >> 54) & 1) {
    out.Y *= -1;
  }

  if ((data >> 55) & 1) {
    out.Z *= -1;
  }
}

void Buf_SphericalRotation::GetFrame(int32 &currentFrame) const {
  currentFrame += data >> 56;
}

int32 Buf_SphericalRotation::GetFrame() const { return data >> 56; }

void Buf_SphericalRotation::SetFrame(uint64 frame) {
  data ^= data & frameField;
  data |= frame << 56;
}

void Buf_SphericalRotation::Interpolate(Vector4A16 &out,
                                        const Buf_SphericalRotation &rightFrame,
                                        float delta,
                                        const TrackMinMax &) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = slerp(startPoint, endPoint, delta);
}

void Buf_SphericalRotation::SwapEndian() { FByteswapper(data); }

size_t Buf_BiLinearVector3_16bit::Size() const { return 8; }

void Buf_BiLinearVector3_16bit::AppendToString(
    std::stringstream &buffer) const {
  AppendToStringRaw(this, buffer);
}

std::string_view
Buf_BiLinearVector3_16bit::RetreiveFromString(std::string_view buffer) {
  return RetreiveFromRawString(this, buffer);
}

void Buf_BiLinearVector3_16bit::Evaluate(Vector4A16 &out) const {
  out = Vector4A16(data.Convert<float>(), 1.0f) * componentMultiplier;
}

void Buf_BiLinearVector3_16bit::Devaluate(const Vector4A16 &in) {
  data = USVector4((in * componentMultiplierInv).Convert<uint16>());
}

void Buf_BiLinearVector3_16bit::GetFrame(int32 &currentFrame) const {
  currentFrame += additiveFrames;
}

int32 Buf_BiLinearVector3_16bit::GetFrame() const { return additiveFrames; }

void Buf_BiLinearVector3_16bit::SetFrame(uint64 frame) {
  additiveFrames = static_cast<uint16>(frame);
}

void Buf_BiLinearVector3_16bit::Interpolate(
    Vector4A16 &out, const Buf_BiLinearVector3_16bit &rightFrame, float delta,
    const TrackMinMax &minMax) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = blerp(startPoint, endPoint, minMax, delta);
}

void Buf_BiLinearVector3_16bit::SwapEndian() {
  FByteswapper(data);
  FByteswapper(additiveFrames);
}

constexpr float __Buf_BiLinearVector3_16bit_componentMask =
    static_cast<float>(Buf_BiLinearVector3_16bit::componentMask);

const Vector4A16 Buf_BiLinearVector3_16bit::componentMultiplierInv =
    Vector4A16(__Buf_BiLinearVector3_16bit_componentMask,
               __Buf_BiLinearVector3_16bit_componentMask,
               __Buf_BiLinearVector3_16bit_componentMask, 1.0f);

const Vector4A16 Buf_BiLinearVector3_16bit::componentMultiplier =
    Vector4A16(1.0f) / componentMultiplierInv;

size_t Buf_BiLinearVector3_8bit::Size() const { return 4; }

void Buf_BiLinearVector3_8bit::AppendToString(std::stringstream &buffer) const {
  AppendToStringRaw(this, buffer);
}

std::string_view
Buf_BiLinearVector3_8bit::RetreiveFromString(std::string_view buffer) {
  return RetreiveFromRawString(this, buffer);
}

void Buf_BiLinearVector3_8bit::Evaluate(Vector4A16 &out) const {
  out = Vector4A16(data.Convert<float>(), 1.0f) * componentMultiplier;
}

void Buf_BiLinearVector3_8bit::Devaluate(const Vector4A16 &in) {
  data = UCVector4((in * componentMultiplierInv).Convert<uint8>());
}

void Buf_BiLinearVector3_8bit::GetFrame(int32 &currentFrame) const {
  currentFrame += additiveFrames;
}

int32 Buf_BiLinearVector3_8bit::GetFrame() const { return additiveFrames; }

void Buf_BiLinearVector3_8bit::SetFrame(uint64 frame) {
  additiveFrames = static_cast<uint8>(frame);
}

void Buf_BiLinearVector3_8bit::Interpolate(
    Vector4A16 &out, const Buf_BiLinearVector3_8bit &rightFrame, float delta,
    const TrackMinMax &minMax) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = blerp(startPoint, endPoint, minMax, delta);
}

constexpr float __Buf_BiLinearVector3_8bit_componentMask =
    static_cast<float>(Buf_BiLinearVector3_8bit::componentMask);

const Vector4A16 Buf_BiLinearVector3_8bit::componentMultiplierInv =
    Vector4A16(__Buf_BiLinearVector3_8bit_componentMask,
               __Buf_BiLinearVector3_8bit_componentMask,
               __Buf_BiLinearVector3_8bit_componentMask, 1.0f);

const Vector4A16 Buf_BiLinearVector3_8bit::componentMultiplier =
    Vector4A16(1.0f) / componentMultiplierInv;

void Buf_LinearRotationQuat4_14bit::Evaluate(Vector4A16 &out) const {
  out = IVector4A16(static_cast<int32>(data >> 42),
                    static_cast<int32>(data >> 28),
                    static_cast<int32>(data >> 14), static_cast<int32>(data)) &
        componentMask;

  const Vector4A16 signedHalf(Vector4A16(static_cast<float>(componentMask)) -
                              out);
  const Vector4A16 multSign(out.X > componentSignMax ? -1.0f : 0.0f,
                            out.Y > componentSignMax ? -1.0f : 0.0f,
                            out.Z > componentSignMax ? -1.0f : 0.0f,
                            out.W > componentSignMax ? -1.0f : 0.0f);

  out *= multSign + 1.0f;
  out += signedHalf * multSign;
  out *= componentMultiplier;
}

void Buf_LinearRotationQuat4_14bit::Devaluate(const Vector4A16 &_in) {
  Vector4A16 in = _in;
  data ^= data & dataField;

  in *= componentMultiplierInv;

  if (in.W < 0.0f) {
    data |= componentMask - static_cast<uint64>(-in.W);
  } else {
    data |= static_cast<uint64>(in.W);
  }

  if (in.Z < 0.0f) {
    data |= (componentMask - static_cast<uint64>(-in.Z)) << 14;
  } else {
    data |= static_cast<uint64>(in.Z) << 14;
  }

  if (in.Y < 0.0f) {
    data |= (componentMask - static_cast<uint64>(-in.Y)) << 28;
  } else {
    data |= static_cast<uint64>(in.Y) << 28;
  }

  if (in.X < 0.0f) {
    data |= (componentMask - static_cast<uint64>(-in.X)) << 42;
  } else {
    data |= static_cast<uint64>(in.X) << 42;
  }
}

void Buf_LinearRotationQuat4_14bit::Interpolate(
    Vector4A16 &out, const Buf_LinearRotationQuat4_14bit &rightFrame,
    float delta, const TrackMinMax &) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = slerp(startPoint, endPoint, delta);
}

size_t Buf_BiLinearRotationQuat4_7bit::Size() const { return 4; }

void Buf_BiLinearRotationQuat4_7bit::AppendToString(
    std::stringstream &buffer) const {
  AppendToStringRaw(this, buffer);
}

std::string_view
Buf_BiLinearRotationQuat4_7bit::RetreiveFromString(std::string_view buffer) {
  return RetreiveFromRawString(this, buffer);
}

void Buf_BiLinearRotationQuat4_7bit::Evaluate(Vector4A16 &out) const {
  out = IVector4A16(data >> 21, data >> 14, data >> 7, data) & componentMask;
  out *= componentMultiplier;
}

void Buf_BiLinearRotationQuat4_7bit::Devaluate(const Vector4A16 &in) {
  data ^= data & dataField;

  UIVector4A16 store(IVector4A16(in * componentMultiplierInv));

  data |= store.W;
  data |= store.Z << 7;
  data |= store.Y << 14;
  data |= store.X << 21;
}

void Buf_BiLinearRotationQuat4_7bit::GetFrame(int32 &currentFrame) const {
  currentFrame += data >> 28;
}

int32 Buf_BiLinearRotationQuat4_7bit::GetFrame() const { return data >> 28; }

void Buf_BiLinearRotationQuat4_7bit::SetFrame(uint64 frame) {
  data ^= data & frameField;
  data |= frame << 28;
}

void Buf_BiLinearRotationQuat4_7bit::Interpolate(
    Vector4A16 &out, const Buf_BiLinearRotationQuat4_7bit &rightFrame,
    float delta, const TrackMinMax &minMax) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = bslerp(startPoint, endPoint, minMax, delta);
}

void Buf_BiLinearRotationQuat4_7bit::SwapEndian() { FByteswapper(data); }

void Buf_BiLinearRotationQuatXW_14bit::Evaluate(Vector4A16 &out) const {
  out = IVector4A16(data, 0, 0, data >> 14) & componentMask;
  out *= componentMultiplier;
}

void Buf_BiLinearRotationQuatXW_14bit::Devaluate(const Vector4A16 &in) {
  data ^= data & dataField;

  UIVector4A16 store(IVector4A16(in * componentMultiplierInv));

  data |= store.X;
  data |= store.W << 14;
}

void Buf_BiLinearRotationQuatXW_14bit::Interpolate(
    Vector4A16 &out, const Buf_BiLinearRotationQuatXW_14bit &rightFrame,
    float delta, const TrackMinMax &minMax) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = bslerp(startPoint, endPoint, minMax, delta);
}

void Buf_BiLinearRotationQuatYW_14bit::Evaluate(Vector4A16 &out) const {
  out = IVector4A16(0, data, 0, data >> 14) & componentMask;
  out *= componentMultiplier;
}

void Buf_BiLinearRotationQuatYW_14bit::Devaluate(const Vector4A16 &in) {
  data ^= data & dataField;

  UIVector4A16 store(IVector4A16(in * componentMultiplierInv));

  data |= store.Y;
  data |= store.W << 14;
}

void Buf_BiLinearRotationQuatYW_14bit::Interpolate(
    Vector4A16 &out, const Buf_BiLinearRotationQuatYW_14bit &rightFrame,
    float delta, const TrackMinMax &minMax) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = bslerp(startPoint, endPoint, minMax, delta);
}

void Buf_BiLinearRotationQuatZW_14bit::Evaluate(Vector4A16 &out) const {
  out = IVector4A16(0, 0, data, data >> 14) & componentMask;
  out *= componentMultiplier;
}

void Buf_BiLinearRotationQuatZW_14bit::Devaluate(const Vector4A16 &in) {
  data ^= data & dataField;

  UIVector4A16 store(IVector4A16(in * componentMultiplierInv));

  data |= store.Z;
  data |= store.W << 14;
}

void Buf_BiLinearRotationQuatZW_14bit::Interpolate(
    Vector4A16 &out, const Buf_BiLinearRotationQuatZW_14bit &rightFrame,
    float delta, const TrackMinMax &minMax) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = bslerp(startPoint, endPoint, minMax, delta);
}

size_t Buf_BiLinearRotationQuat4_11bit::Size() const { return 6; }

void Buf_BiLinearRotationQuat4_11bit::AppendToString(
    std::stringstream &buffer) const {
  AppendToStringRaw(this, buffer);
}

std::string_view
Buf_BiLinearRotationQuat4_11bit::RetreiveFromString(std::string_view buffer) {
  return RetreiveFromRawString(this, buffer);
}

void Buf_BiLinearRotationQuat4_11bit::Evaluate(Vector4A16 &out) const {
  const uint64 &rVal = reinterpret_cast<const uint64 &>(data);

  out = IVector4A16(static_cast<int32>(rVal),
                    static_cast<int32>(((rVal >> 11) << 6) | (data[1] & 0x3f)),
                    static_cast<int32>((rVal >> 22) << 1 | (data[2] & 1)),
                    static_cast<int32>(rVal >> 33)) &
        componentMask;
  out *= componentMultiplier;
}

void Buf_BiLinearRotationQuat4_11bit::Devaluate(const Vector4A16 &in) {
  uint64 &rVal = reinterpret_cast<uint64 &>(data);

  rVal ^= rVal & 0xFFFFFFFFFFF;

  t_Vector4<uint64> store = (in * componentMultiplierInv).Convert<uint64>();

  rVal |= store.X;
  rVal |= (store.Y >> 6 | (store.Y & 0x3f) << 5) << 11;
  rVal |= (store.Z >> 1 | (store.Z & 1) << 10) << 22;
  rVal |= store.W << 33;
}

void Buf_BiLinearRotationQuat4_11bit::GetFrame(int32 &currentFrame) const {
  currentFrame += data.Z >> 12;
}

void Buf_BiLinearRotationQuat4_11bit::Interpolate(
    Vector4A16 &out, const Buf_BiLinearRotationQuat4_11bit &rightFrame,
    float delta, const TrackMinMax &minMax) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = bslerp(startPoint, endPoint, minMax, delta);
}

void Buf_BiLinearRotationQuat4_11bit::SwapEndian() { FByteswapper(data); }

size_t Buf_BiLinearRotationQuat4_9bit::Size() const { return 5; }

void Buf_BiLinearRotationQuat4_9bit::AppendToString(
    std::stringstream &buffer) const {
  AppendToStringRaw(this, buffer);
}

std::string_view
Buf_BiLinearRotationQuat4_9bit::RetreiveFromString(std::string_view buffer) {
  return RetreiveFromRawString(this, buffer);
}

void Buf_BiLinearRotationQuat4_9bit::Evaluate(Vector4A16 &out) const {
  const uint64 &rVal = reinterpret_cast<const uint64 &>(data);

  out = IVector4A16(static_cast<int32>((rVal << 1) | (data[1] & 1)),
                    static_cast<int32>(((rVal >> 9) << 2) | (data[2] & 3)),
                    static_cast<int32>(((rVal >> 18) << 3) | (data[3] & 7)),
                    static_cast<int32>(((rVal >> 27) << 4) | (data[4] & 0xf))) &
        componentMask;
  out *= componentMultiplier;
}

void Buf_BiLinearRotationQuat4_9bit::Devaluate(const Vector4A16 &in) {
  uint64 &rVal = reinterpret_cast<uint64 &>(data);

  rVal ^= rVal & 0xFFFFFFFFF;

  t_Vector4<uint64> store = (in * componentMultiplierInv).Convert<uint64>();

  rVal |= store.X >> 1 | (store.X & 1) << 8;
  rVal |= (store.Y >> 2 | (store.Y & 3) << 7) << 9;
  rVal |= (store.Z >> 3 | (store.Z & 7) << 6) << 18;
  rVal |= (store.W >> 4 | (store.W & 0xf) << 5) << 27;
}

void Buf_BiLinearRotationQuat4_9bit::GetFrame(int32 &currentFrame) const {
  currentFrame += data[4] >> 4;
}

void Buf_BiLinearRotationQuat4_9bit::Interpolate(
    Vector4A16 &out, const Buf_BiLinearRotationQuat4_9bit &rightFrame,
    float delta, const TrackMinMax &minMax) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = bslerp(startPoint, endPoint, minMax, delta);
}

template <class C>
void Buff_EvalShared<C>::ToString(std::string &strBuff,
                                  size_t numIdents) const {
  static const char *idents[] = {
      "", "\t", "\t\t", "\t\t\t", "\t\t\t\t", "\t\t\t\t\t",
  };
  std::stringstream str;
  str << std::endl << idents[numIdents];

  size_t curLine = 1;

  for (auto &d : data) {
    d.AppendToString(str);

    if (!(curLine % C::NEWLINEMOD)) {
      str << std::endl << idents[numIdents];
    }

    curLine++;
  }

  if (!((curLine - 1) % C::NEWLINEMOD)) {
    str.seekp(-1, std::ios_base::cur) << '\0';
  } else {
    str << std::endl << idents[numIdents - 1];
  }

  strBuff = std::move(str).str();
}

template <class C> void Buff_EvalShared<C>::FromString(std::string_view input) {
  for (auto &d : data) {
    input = d.RetreiveFromString(input);
  }
}

template <class C>
void Buff_EvalShared<C>::Assign(char *ptr, size_t size, bool swapEndian) {
  if constexpr (!C::VARIABLE_SIZE) {
    data = {reinterpret_cast<C *>(ptr), reinterpret_cast<C *>(ptr + size)};
  } else {
    const char *bufferEnd = ptr + size;

    while (ptr < bufferEnd) {
      C *block = reinterpret_cast<C *>(ptr);
      internalData.push_back(*block);
      ptr += block->Size();
    }

    data = internalData;
  }

  if (swapEndian) {
    SwapEndian();
  }

  frames.resize(NumFrames());
  int32 currentFrame = 0;
  size_t curFrameID = 0;

  for (auto &f : frames) {
    f = currentFrame;
    data[curFrameID++].GetFrame(currentFrame);
  }
}

template <class C> void Buff_EvalShared<C>::Save(BinWritterRef wr) const {
  if constexpr (!C::VARIABLE_SIZE) {
    if (!wr.SwappedEndian()) {
      wr.WriteBuffer(reinterpret_cast<const char *>(data.data()),
                     data.size() * sizeof(typename decltype(data)::value_type));
    } else {
      wr.WriteContainer(data);
    }
  } else {
    for (auto &r : data) {
      C tmp = r;
      tmp.SwapEndian();
      wr.WriteBuffer(reinterpret_cast<const char *>(&tmp), r.Size());
    }
  }
}

template <class C> void Buff_EvalShared<C>::SwapEndian() {
  for (auto &d : data) {
    d.SwapEndian();
  }
}

template <class Derived> LMTTrackController *_creatorDummyBuffEval() {
  return new Buff_EvalShared<Derived>();
}

#define TCON_REG(val) {TrackTypesShared::val, _creatorDummyBuffEval<Buf_##val>},

#define TCONFRC_REG(val)                                                       \
  {TrackTypesShared::val, [] { return Buf_##val::componentMultiplier; }()},

namespace std {
template <> struct hash<TrackTypesShared> {
  uint32 operator()(const TrackTypesShared &t) const {
    return static_cast<uint32>(t);
  }
};
} // namespace std

static const std::unordered_map<TrackTypesShared, LMTTrackController *(*)()>
    codecRegistry = {
        {TrackTypesShared::None, nullptr},
        StaticFor(TCON_REG, SingleVector3, HermiteVector3, StepRotationQuat3,
                  SphericalRotation, LinearVector3, BiLinearVector3_16bit,
                  BiLinearVector3_8bit, LinearRotationQuat4_14bit,
                  BiLinearRotationQuat4_7bit, BiLinearRotationQuatXW_14bit,
                  BiLinearRotationQuatYW_14bit, BiLinearRotationQuatZW_14bit,
                  BiLinearRotationQuat4_11bit, BiLinearRotationQuat4_9bit)};

static const std::unordered_map<TrackTypesShared, float> fracRegistry = {
    {TrackTypesShared::BiLinearVector3_16bit,
     Buf_BiLinearVector3_16bit::componentMultiplier.X},
    {TrackTypesShared::BiLinearVector3_8bit,
     Buf_BiLinearVector3_8bit::componentMultiplier.X},
    StaticFor(TCONFRC_REG, LinearRotationQuat4_14bit,
              BiLinearRotationQuat4_7bit, BiLinearRotationQuatXW_14bit,
              BiLinearRotationQuatYW_14bit, BiLinearRotationQuatZW_14bit,
              BiLinearRotationQuat4_11bit, BiLinearRotationQuat4_9bit)};

LMTTrackController *LMTTrackController::CreateCodec(TrackTypesShared cType) {
  if (codecRegistry.count(cType)) {
    return codecRegistry.at(cType)();
  }

  return nullptr;
}

float LMTTrackController::GetTrackMaxFrac(TrackTypesShared type) {
  if (fracRegistry.count(type)) {
    return fracRegistry.at(type);
  }

  return FLT_MAX;
}
