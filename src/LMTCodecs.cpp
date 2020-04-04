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

#include "LMTCodecs.h"
#include "datas/binwritter.hpp"
#include "datas/macroLoop.hpp"
#include "datas/masterprinter.hpp"
#include "datas/reflectorRegistry.hpp"
#include <cmath>
#include <sstream>
#include <unordered_map>

REFLECTOR_CREATE(Buf_SingleVector3, 1, VARNAMES, data);
REFLECTOR_CREATE(Buf_LinearVector3, 1, VARNAMES, data, additiveFrames);
REFLECTOR_CREATE(Buf_HermiteVector3, 1, VARNAMES, flags, additiveFrames, data);

static const float fPI = 3.14159265;
static const float fPI2 = 0.5 * fPI;

// https://en.wikipedia.org/wiki/Slerp
ES_INLINE Vector4A16 slerp(const Vector4A16 &v0, const Vector4A16 &_v1, float t) {
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

  return (v0 * s0) + (v1 * s1);
}

template <typename T>
ES_INLINE Vector4A16 lerp(const Vector4A16 &v0, const Vector4A16 &v1, T t) {
  return v0 + (v1 - v0) * t;
}

ES_INLINE Vector4A16 additiveLerp(const TrackMinMax *minMax,
                                  const Vector4A16 &value) {
  return minMax->max + minMax->min * value;
}

ES_INLINE Vector4A16 blerp(const Vector4A16 &v0, const Vector4A16 &v1,
                           const TrackMinMax *minMax, float t) {
  return lerp(additiveLerp(minMax, v0), additiveLerp(minMax, v1), t);
}

ES_INLINE Vector4A16 bslerp(const Vector4A16 &v0, const Vector4A16 &v1,
                            const TrackMinMax *minMax, float t) {
  return slerp(additiveLerp(minMax, v0), additiveLerp(minMax, v1), t);
}

template <class C>
void AppendToStringRaw(const C *clPtr, std::stringstream &buffer) {
  const uchar *rawData = reinterpret_cast<const uchar *>(clPtr);
  const int hexSize = clPtr->Size() * 2;
  int cBuff = 0;

  for (int i = 0; i < hexSize; i++) {
    bool idk = i & 1;
    char temp = 0x30 + (idk ? rawData[cBuff++] & 0xf : rawData[cBuff] >> 4);

    if (temp > 0x39)
      temp += 7;

    buffer << temp;
  }
}

template <class C>
void RetreiveFromRawString(C *clPtr, const std::string &buffer,
                           int &bufferIter) {
  uchar *rawData = reinterpret_cast<uchar *>(clPtr);
  const int hexSize = clPtr->Size() * 2;
  int cBuff = 0;

  for (int i = 0; i < hexSize; i++, bufferIter++) {
    if (bufferIter >= buffer.size()) {
      bufferIter = -1;
      return;
    }

    const char &cRef = buffer.at(bufferIter);

    if (cRef < '0' || cRef > 'F' || (cRef > '9' && cRef < 'A')) {
      i--;
      continue;
    }

    bool idk = i & 1;

    if (!idk)
      rawData[cBuff] = atohLUT[cRef] << 4;
    else
      rawData[cBuff++] |= atohLUT[cRef];
  }
}

static void ltrim(const std::string &s, int &curIterPos) {
  for (auto it = s.begin() + curIterPos; it != s.end(); it++) {
    switch (*it) {
    case '\n':
    case '\t':
    case ' ':
      curIterPos++;
      break;
    default:
      return;
    }
  }
}

ES_INLINE void SeekTo(const std::string &s, int &curIterPos,
                      const char T = '\n') {
  for (auto it = s.begin() + curIterPos; it != s.end(); it++) {
    curIterPos++;

    if (*it == T)
      return;
  }
}

int Buf_SingleVector3::Size() const { return 12; }

void Buf_SingleVector3::AppendToString(std::stringstream &buffer) const {
  ReflectorWrapConst<Buf_SingleVector3> tRefl(this);

  buffer << tRefl.GetReflectedValue(0);
}

void Buf_SingleVector3::RetreiveFromString(const std::string &buffer,
                                           int &bufferIter) {
  ltrim(buffer, bufferIter);

  ReflectorWrap<Buf_SingleVector3> tRefl(this);
  tRefl.SetReflectedValue(0, buffer.c_str() + bufferIter);
  SeekTo(buffer, bufferIter);
}

void Buf_SingleVector3::Evaluate(Vector4A16 &out) const {
  out = Vector4A16(data, 1.0f);
}

void Buf_SingleVector3::Devaluate(const Vector4A16 &in) { data = in; }

void Buf_SingleVector3::GetFrame(int &currentFrame) const { currentFrame++; }

int Buf_SingleVector3::GetFrame() const { return 1; }

void Buf_SingleVector3::Interpolate(Vector4A16 &out,
                                   const Buf_SingleVector3 &rightFrame,
                                   float delta, const TrackMinMax *) const {
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
                                       float delta, const TrackMinMax *) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = slerp(startPoint, endPoint, delta);
}

int Buf_LinearVector3::Size() const { return 16; }

void Buf_LinearVector3::AppendToString(std::stringstream &buffer) const {
  ReflectorWrapConst<Buf_LinearVector3> tRefl(this);

  buffer << "{ " << tRefl.GetReflectedValue(0) << ", "
         << tRefl.GetReflectedValue(1) << " }";
}

void Buf_LinearVector3::RetreiveFromString(const std::string &buffer,
                                           int &bufferIter) {
  SeekTo(buffer, bufferIter, '{');
  ltrim(buffer, bufferIter);

  ReflectorWrap<Buf_LinearVector3> tRefl(this);
  tRefl.SetReflectedValue(0, buffer.c_str() + bufferIter);

  SeekTo(buffer, bufferIter, ']');
  SeekTo(buffer, bufferIter, ',');
  ltrim(buffer, bufferIter);

  tRefl.SetReflectedValue(1, buffer.c_str() + bufferIter);

  SeekTo(buffer, bufferIter);
}

void Buf_LinearVector3::Devaluate(const Vector4A16 &in) { data = in; }

void Buf_LinearVector3::Evaluate(Vector4A16 &out) const {
  out = Vector4A16(data, 1.0f);
}

void Buf_LinearVector3::GetFrame(int &currentFrame) const {
  currentFrame += additiveFrames;
}

int Buf_LinearVector3::GetFrame() const { return additiveFrames; }

void Buf_LinearVector3::SetFrame(uint64 frame) {
  additiveFrames = static_cast<int>(frame);
}

void Buf_LinearVector3::Interpolate(Vector4A16 &out,
                                   const Buf_LinearVector3 &rightFrame,
                                   float delta, const TrackMinMax *) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = lerp(startPoint, endPoint, delta);
}

void Buf_LinearVector3::SwapEndian() {
  FByteswapper(data);
  FByteswapper(additiveFrames);
}

int Buf_HermiteVector3::Size() const { return size; }

void Buf_HermiteVector3::AppendToString(std::stringstream &buffer) const {
  ReflectorWrapConst<Buf_HermiteVector3> tRefl(this);

  buffer << "{ " << tRefl.GetReflectedValue(2) << ", "
         << tRefl.GetReflectedValue(1) << ", " << tRefl.GetReflectedValue(0);

  int curTang = 0;

  for (int f = 0; f < 6; f++)
    if (flags[static_cast<Buf_HermiteVector3_Flags>(f)]) {
      buffer << ", " << tangents[curTang++];
    }

  buffer << " }";
}

void Buf_HermiteVector3::RetreiveFromString(const std::string &buffer,
                                            int &bufferIter) {
  SeekTo(buffer, bufferIter, '{');
  ltrim(buffer, bufferIter);

  ReflectorWrap<Buf_HermiteVector3> tRefl(this);
  tRefl.SetReflectedValue(2, buffer.c_str() + bufferIter);

  SeekTo(buffer, bufferIter, ']');
  SeekTo(buffer, bufferIter, ',');
  ltrim(buffer, bufferIter);

  tRefl.SetReflectedValue(1, buffer.c_str() + bufferIter);

  SeekTo(buffer, bufferIter, ',');
  ltrim(buffer, bufferIter);

  tRefl.SetReflectedValue(0, buffer.c_str() + bufferIter);

  SeekTo(buffer, bufferIter, ',');
  ltrim(buffer, bufferIter);

  int curTang = 0;
  int lastIter = bufferIter;

  for (int f = 0; f < 6; f++)
    if (flags[static_cast<Buf_HermiteVector3_Flags>(f)]) {
      tangents[curTang++] = std::atof(buffer.c_str() + bufferIter);
      lastIter = bufferIter;
      SeekTo(buffer, bufferIter, ',');
      ltrim(buffer, bufferIter);
    }

  bufferIter = lastIter;

  size = sizeof(Buf_HermiteVector3) - (6 - curTang) * 4;

  SeekTo(buffer, bufferIter);
}

void Buf_HermiteVector3::Evaluate(Vector4A16 &out) const {
  out = Vector4A16(data, 1.0f);
}

void Buf_HermiteVector3::GetTangents(Vector4A16 &inTangs,
                                     Vector4A16 &outTangs) const {
  int currentTangIndex = 0;

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

void Buf_HermiteVector3::GetFrame(int &currentFrame) const {
  currentFrame += additiveFrames;
}

int Buf_HermiteVector3::GetFrame() const { return additiveFrames; }

void Buf_HermiteVector3::SetFrame(uint64 frame) {
  additiveFrames = static_cast<short>(frame);
}

void Buf_HermiteVector3::Interpolate(Vector4A16 &out,
                                    const Buf_HermiteVector3 &rightFrame,
                                    float delta, const TrackMinMax *) const {
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

  int curTang = 0;

  for (int f = 0; f < 6; f++)
    if (flags[static_cast<Buf_HermiteVector3_Flags>(f)]) {
      FByteswapper(tangents[curTang]);
      curTang++;
    }
}

int Buf_SphericalRotation::Size() const { return 8; }

void Buf_SphericalRotation::AppendToString(std::stringstream &buffer) const {
  AppendToStringRaw(this, buffer);
}

void Buf_SphericalRotation::RetreiveFromString(const std::string &buffer,
                                               int &bufferIter) {
  RetreiveFromRawString(this, buffer, bufferIter);
}

void Buf_SphericalRotation::Devaluate(const Vector4A16 &_in) {
  Vector4A16 in = _in;
  data ^= data & dataField;

  if (in.W < 0.0f)
    in = -in;

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
  out.X = data & componentMask;
  out.Y = (data >> 17) & componentMask;
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

  if ((data >> 53) & 1)
    out.X *= -1;

  if ((data >> 54) & 1)
    out.Y *= -1;

  if ((data >> 55) & 1)
    out.Z *= -1;
}

void Buf_SphericalRotation::GetFrame(int &currentFrame) const {
  currentFrame += data >> 56;
}

int Buf_SphericalRotation::GetFrame() const { return data >> 56; }

void Buf_SphericalRotation::SetFrame(uint64 frame) {
  data ^= data & frameField;
  data |= frame << 56;
}

void Buf_SphericalRotation::Interpolate(Vector4A16 &out,
                                       const Buf_SphericalRotation &rightFrame,
                                       float delta,
                                       const TrackMinMax *minMax) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = slerp(startPoint, endPoint, delta);
}

void Buf_SphericalRotation::SwapEndian() { FByteswapper(data); }

const float Buf_SphericalRotation::componentMultiplierInv =
    static_cast<float>(componentMask) / fPI2;
const float Buf_SphericalRotation::componentMultiplier =
    1.0f / componentMultiplierInv;
const float Buf_SphericalRotation::componentMultiplierW =
    1.0f / static_cast<float>(componentMaskW);

int Buf_BiLinearVector3_16bit::Size() const { return 8; }

void Buf_BiLinearVector3_16bit::AppendToString(
    std::stringstream &buffer) const {
  AppendToStringRaw(this, buffer);
}

void Buf_BiLinearVector3_16bit::RetreiveFromString(const std::string &buffer,
                                                   int &bufferIter) {
  RetreiveFromRawString(this, buffer, bufferIter);
}

void Buf_BiLinearVector3_16bit::Evaluate(Vector4A16 &out) const {
  out = Vector4A16(data.Convert<float>(), 1.0f) * componentMultiplier;
}

void Buf_BiLinearVector3_16bit::Devaluate(const Vector4A16 &in) {
  data = USVector4((in * componentMultiplierInv).Convert<ushort>());
}

void Buf_BiLinearVector3_16bit::GetFrame(int &currentFrame) const {
  currentFrame += additiveFrames;
}

int Buf_BiLinearVector3_16bit::GetFrame() const { return additiveFrames; }

void Buf_BiLinearVector3_16bit::SetFrame(uint64 frame) {
  additiveFrames = static_cast<ushort>(frame);
}

void Buf_BiLinearVector3_16bit::Interpolate(
    Vector4A16 &out, const Buf_BiLinearVector3_16bit &rightFrame, float delta,
    const TrackMinMax *minMax) const {
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

int Buf_BiLinearVector3_8bit::Size() const { return 4; }

void Buf_BiLinearVector3_8bit::AppendToString(std::stringstream &buffer) const {
  AppendToStringRaw(this, buffer);
}

void Buf_BiLinearVector3_8bit::RetreiveFromString(const std::string &buffer,
                                                  int &bufferIter) {
  RetreiveFromRawString(this, buffer, bufferIter);
}

void Buf_BiLinearVector3_8bit::Evaluate(Vector4A16 &out) const {
  out = Vector4A16(data.Convert<float>(), 1.0f) * componentMultiplier;
}

void Buf_BiLinearVector3_8bit::Devaluate(const Vector4A16 &in) {
  data = UCVector4((in * componentMultiplierInv).Convert<uchar>());
}

void Buf_BiLinearVector3_8bit::GetFrame(int &currentFrame) const {
  currentFrame += additiveFrames;
}

int Buf_BiLinearVector3_8bit::GetFrame() const { return additiveFrames; }

void Buf_BiLinearVector3_8bit::SetFrame(uint64 frame) {
  additiveFrames = static_cast<ushort>(frame);
}

void Buf_BiLinearVector3_8bit::Interpolate(
    Vector4A16 &out, const Buf_BiLinearVector3_8bit &rightFrame, float delta,
    const TrackMinMax *minMax) const {
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
  out = IVector4A16(data >> 42, data >> 28, data >> 14, data) & componentMask;

  const Vector4A16 signedHalf(Vector4A16(componentMask) - out);
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

  if (in.W < 0.0f)
    data |= componentMask - static_cast<uint64>(-in.W);
  else
    data |= static_cast<uint64>(in.W);

  if (in.Z < 0.0f)
    data |= (componentMask - static_cast<uint64>(-in.Z)) << 14;
  else
    data |= static_cast<uint64>(in.Z) << 14;

  if (in.Y < 0.0f)
    data |= (componentMask - static_cast<uint64>(-in.Y)) << 28;
  else
    data |= static_cast<uint64>(in.Y) << 28;

  if (in.X < 0.0f)
    data |= (componentMask - static_cast<uint64>(-in.X)) << 42;
  else
    data |= static_cast<uint64>(in.X) << 42;
}

void Buf_LinearRotationQuat4_14bit::Interpolate(
    Vector4A16 &out, const Buf_LinearRotationQuat4_14bit &rightFrame,
    float delta, const TrackMinMax *minMax) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = slerp(startPoint, endPoint, delta);
}

const float Buf_LinearRotationQuat4_14bit::componentMultiplierInv =
    static_cast<float>(componentMask) / 4.0f;
const float Buf_LinearRotationQuat4_14bit::componentMultiplier =
    1.0f / componentMultiplierInv;
const float Buf_LinearRotationQuat4_14bit::componentSignMax =
    static_cast<float>(componentMask) * 0.5;

int Buf_BiLinearRotationQuat4_7bit::Size() const { return 4; }

void Buf_BiLinearRotationQuat4_7bit::AppendToString(
    std::stringstream &buffer) const {
  AppendToStringRaw(this, buffer);
}

void Buf_BiLinearRotationQuat4_7bit::RetreiveFromString(
    const std::string &buffer, int &bufferIter) {
  RetreiveFromRawString(this, buffer, bufferIter);
}

void Buf_BiLinearRotationQuat4_7bit::Evaluate(Vector4A16 &out) const {
  out = IVector4A16(data >> 21, data >> 14, data >> 7, data) & componentMask;
  out *= componentMultiplier;
}

void Buf_BiLinearRotationQuat4_7bit::Devaluate(const Vector4A16 &in) {
  data ^= data & dataField;

  UIVector4A16 store(in * componentMultiplierInv);

  data |= store.W;
  data |= store.Z << 7;
  data |= store.Y << 14;
  data |= store.X << 21;
}

void Buf_BiLinearRotationQuat4_7bit::GetFrame(int &currentFrame) const {
  currentFrame += data >> 28;
}

int Buf_BiLinearRotationQuat4_7bit::GetFrame() const { return data >> 28; }

void Buf_BiLinearRotationQuat4_7bit::SetFrame(uint64 frame) {
  data ^= data & frameField;
  data |= frame << 28;
}

void Buf_BiLinearRotationQuat4_7bit::Interpolate(
    Vector4A16 &out, const Buf_BiLinearRotationQuat4_7bit &rightFrame,
    float delta, const TrackMinMax *minMax) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = bslerp(startPoint, endPoint, minMax, delta);
}

void Buf_BiLinearRotationQuat4_7bit::SwapEndian() { FByteswapper(data); }

const float Buf_BiLinearRotationQuat4_7bit::componentMultiplierInv =
    static_cast<float>(componentMask);
const float Buf_BiLinearRotationQuat4_7bit::componentMultiplier =
    1.0f / componentMultiplierInv;

void Buf_BiLinearRotationQuatXW_14bit::Evaluate(Vector4A16 &out) const {
  out = IVector4A16(data, 0, 0, data >> 14) & componentMask;
  out *= componentMultiplier;
}

void Buf_BiLinearRotationQuatXW_14bit::Devaluate(const Vector4A16 &in) {
  data ^= data & dataField;

  UIVector4A16 store(in * componentMultiplierInv);

  data |= store.X;
  data |= store.W << 14;
}

void Buf_BiLinearRotationQuatXW_14bit::Interpolate(
    Vector4A16 &out, const Buf_BiLinearRotationQuatXW_14bit &rightFrame,
    float delta, const TrackMinMax *minMax) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = bslerp(startPoint, endPoint, minMax, delta);
}

const float Buf_BiLinearRotationQuatXW_14bit::componentMultiplierInv =
    static_cast<float>(componentMask);
const float Buf_BiLinearRotationQuatXW_14bit::componentMultiplier =
    1.0f / componentMultiplierInv;

void Buf_BiLinearRotationQuatYW_14bit::Evaluate(Vector4A16 &out) const {
  out = IVector4A16(0, data, 0, data >> 14) & componentMask;
  out *= componentMultiplier;
}

void Buf_BiLinearRotationQuatYW_14bit::Devaluate(const Vector4A16 &in) {
  data ^= data & dataField;

  UIVector4A16 store(in * componentMultiplierInv);

  data |= store.Y;
  data |= store.W << 14;
}

void Buf_BiLinearRotationQuatYW_14bit::Interpolate(
    Vector4A16 &out, const Buf_BiLinearRotationQuatYW_14bit &rightFrame,
    float delta, const TrackMinMax *minMax) const {
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

  UIVector4A16 store(in * componentMultiplierInv);

  data |= store.Z;
  data |= store.W << 14;
}

void Buf_BiLinearRotationQuatZW_14bit::Interpolate(
    Vector4A16 &out, const Buf_BiLinearRotationQuatZW_14bit &rightFrame,
    float delta, const TrackMinMax *minMax) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = bslerp(startPoint, endPoint, minMax, delta);
}

int Buf_BiLinearRotationQuat4_11bit::Size() const { return 6; }

void Buf_BiLinearRotationQuat4_11bit::AppendToString(
    std::stringstream &buffer) const {
  AppendToStringRaw(this, buffer);
}

void Buf_BiLinearRotationQuat4_11bit::RetreiveFromString(
    const std::string &buffer, int &bufferIter) {
  RetreiveFromRawString(this, buffer, bufferIter);
}

void Buf_BiLinearRotationQuat4_11bit::Evaluate(Vector4A16 &out) const {
  const uint64 &rVal = reinterpret_cast<const uint64 &>(data);

  out = IVector4A16(rVal, ((rVal >> 11) << 6) | data[1] & 0x3f,
                    (rVal >> 22) << 1 | data[2] & 1, rVal >> 33) &
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

void Buf_BiLinearRotationQuat4_11bit::GetFrame(int &currentFrame) const {
  currentFrame += data.Z >> 12;
}

void Buf_BiLinearRotationQuat4_11bit::Interpolate(
    Vector4A16 &out, const Buf_BiLinearRotationQuat4_11bit &rightFrame,
    float delta, const TrackMinMax *minMax) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = bslerp(startPoint, endPoint, minMax, delta);
}

void Buf_BiLinearRotationQuat4_11bit::SwapEndian() { FByteswapper(data); }

const float Buf_BiLinearRotationQuat4_11bit::componentMultiplierInv =
    static_cast<float>(componentMask);
const float Buf_BiLinearRotationQuat4_11bit::componentMultiplier =
    1.0f / componentMultiplierInv;

int Buf_BiLinearRotationQuat4_9bit::Size() const { return 5; }

void Buf_BiLinearRotationQuat4_9bit::AppendToString(
    std::stringstream &buffer) const {
  AppendToStringRaw(this, buffer);
}

void Buf_BiLinearRotationQuat4_9bit::RetreiveFromString(
    const std::string &buffer, int &bufferIter) {
  RetreiveFromRawString(this, buffer, bufferIter);
}

void Buf_BiLinearRotationQuat4_9bit::Evaluate(Vector4A16 &out) const {
  const uint64 &rVal = reinterpret_cast<const uint64 &>(data);

  out = IVector4A16((rVal << 1) | (data[1] & 1),
                    ((rVal >> 9) << 2) | (data[2] & 3),
                    ((rVal >> 18) << 3) | (data[3] & 7),
                    ((rVal >> 27) << 4) | (data[4] & 0xf)) &
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

void Buf_BiLinearRotationQuat4_9bit::GetFrame(int &currentFrame) const {
  currentFrame += data[4] >> 4;
}

void Buf_BiLinearRotationQuat4_9bit::Interpolate(
    Vector4A16 &out, const Buf_BiLinearRotationQuat4_9bit &rightFrame,
    float delta, const TrackMinMax *minMax) const {
  Vector4A16 startPoint, endPoint;

  Evaluate(startPoint);
  rightFrame.Evaluate(endPoint);

  out = bslerp(startPoint, endPoint, minMax, delta);
}

const float Buf_BiLinearRotationQuat4_9bit::componentMultiplierInv =
    static_cast<float>(componentMask);
const float Buf_BiLinearRotationQuat4_9bit::componentMultiplier =
    1.0f / componentMultiplierInv;

template <class C>
void Buff_EvalShared<C>::ToString(std::string &strBuff, int numIdents) const {
  std::stringstream str;
  str << std::endl << idents[numIdents];

  int curLine = 1;

  for (auto &d : data) {
    d.AppendToString(str);

    if (!(curLine % C::NEWLINEMOD))
      str << std::endl << idents[numIdents];

    curLine++;
  }

  if (!((curLine - 1) % C::NEWLINEMOD))
    str.seekp(-1, std::ios_base::cur) << '\0';
  else
    str << std::endl << idents[numIdents - 1];

  strBuff = str.str();
}

template <class C> void Buff_EvalShared<C>::FromString(std::string &input) {
  int iterPos = 0;

  for (auto &d : data) {
    d.RetreiveFromString(input, iterPos);

    if (iterPos < 0) {
      printerror("[LMT] Unexpected end of <data/> buffer.") return;
    }
  }
}

template <class C> void Buff_EvalShared<C>::Assign(char *ptr, int size) {
  if (!C::VARIABLE_SIZE) {
    data = Store_Type(reinterpret_cast<C *>(ptr),
                      reinterpret_cast<C *>(ptr + size),
                      std::allocator_hybrid<C>(reinterpret_cast<C *>(ptr)));
  } else {
    const char *bufferEnd = ptr + size;

    while (ptr < bufferEnd) {
      C *block = reinterpret_cast<C *>(ptr);
      data.push_back(*block);
      ptr += block->Size();
    }
  }

  frames.resize(NumFrames());
  int currentFrame = 0;
  int curFrameID = 0;

  for (auto &f : frames) {
    f = currentFrame;
    data[curFrameID].GetFrame(currentFrame);
    curFrameID++;
  }
}

template <class C> void Buff_EvalShared<C>::Save(BinWritter *wr) const {
  if (!C::VARIABLE_SIZE) {
    if (!wr->SwappedEndian())
      wr->WriteBuffer(reinterpret_cast<const char *>(data.data()),
                      data.size() * sizeof(typename Store_Type::value_type));
    else
      wr->WriteContainer(data);
  } else {
    for (auto &r : data) {
      C tmp = r;
      tmp.SwapEndian();
      wr->WriteBuffer(reinterpret_cast<const char *>(&tmp), r.Size());
    }
  }
}

template <class C> void Buff_EvalShared<C>::SwapEndian() {
  for (auto &d : data)
    d.SwapEndian();
}

template <class Derived> LMTTrackController *_creatorDummyBuffEval() {
  return new Buff_EvalShared<Derived>();
}

#define TCON_REG(val) {TrackTypesShared::val, _creatorDummyBuffEval<Buf_##val>},

#define TCONFRC_REG(val)                                                       \
  {TrackTypesShared::val, Buf_##val::componentMultiplier},

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

static const TrackTypesShared buffRemapRegistry[][16] = {
    {TrackTypesShared::None, TrackTypesShared::SingleVector3,
     TrackTypesShared::SingleVector3, TrackTypesShared::None,
     TrackTypesShared::StepRotationQuat3, TrackTypesShared::HermiteVector3,
     TrackTypesShared::SphericalRotation, TrackTypesShared::None,
     TrackTypesShared::None, TrackTypesShared::LinearVector3},

    {TrackTypesShared::None, TrackTypesShared::SingleVector3,
     TrackTypesShared::SingleVector3, TrackTypesShared::None,
     TrackTypesShared::StepRotationQuat3, TrackTypesShared::HermiteVector3,
     TrackTypesShared::LinearRotationQuat4_14bit, TrackTypesShared::None,
     TrackTypesShared::None, TrackTypesShared::LinearVector3},

    {TrackTypesShared::None, TrackTypesShared::SingleVector3,
     TrackTypesShared::StepRotationQuat3, TrackTypesShared::LinearVector3,
     TrackTypesShared::BiLinearVector3_16bit,
     TrackTypesShared::BiLinearVector3_8bit,
     TrackTypesShared::LinearRotationQuat4_14bit,
     TrackTypesShared::BiLinearRotationQuat4_7bit, TrackTypesShared::None,
     TrackTypesShared::None, TrackTypesShared::None,
     TrackTypesShared::BiLinearRotationQuatXW_14bit,
     TrackTypesShared::BiLinearRotationQuatYW_14bit,
     TrackTypesShared::BiLinearRotationQuatZW_14bit,
     TrackTypesShared::BiLinearRotationQuat4_11bit,
     TrackTypesShared::BiLinearRotationQuat4_9bit}};

REGISTER_ENUMS(Buf_HermiteVector3_Flags)

LMTTrackController *LMTTrackController::CreateCodec(int type, int subVersion) {
  const TrackTypesShared cType = subVersion < 0
                                     ? static_cast<TrackTypesShared>(type)
                                     : buffRemapRegistry[subVersion][type];
  RegisterLocalEnums();

  if (codecRegistry.count(cType))
    return codecRegistry.at(cType)();

  return nullptr;
}

float LMTTrackController::GetTrackMaxFrac(TrackTypesShared type) {
  if (fracRegistry.count(type))
    return fracRegistry.at(type);

  return FLT_MAX;
}