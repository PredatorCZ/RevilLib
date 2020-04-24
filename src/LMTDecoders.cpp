#include "LMTCodecs.h"
#include "LMTDecode.h"
#include <algorithm>

typedef std::vector<Vector4A16> VectCollection;

static TrackTypesShared Choose2ComponentQuat(const VectCollection &input) {
  bool staticComponent[4] = {true, true, true, false};
  uint32 &staticComponentInt = reinterpret_cast<uint32 &>(staticComponent);
  Vector4A16 firstItem = input[0];
  float componentEpsilon = 0.00001f;

  for (auto &i : input) {
    if (!staticComponentInt)
      return TrackTypesShared::None;

    for (uint32 c = 0; c < 3; c++)
      if (staticComponent[c] && ((i[c] >= firstItem[c] + componentEpsilon) ||
                                 (i[c] <= firstItem[c] - componentEpsilon))) {
        staticComponent[c] = false;
      }
  }

  if (!staticComponent[0])
    return TrackTypesShared::BiLinearRotationQuatXW_14bit;
  else if (!staticComponent[1])
    return TrackTypesShared::BiLinearRotationQuatYW_14bit;
  else if (!staticComponent[3])
    return TrackTypesShared::BiLinearRotationQuatZW_14bit;
  else
    return TrackTypesShared::None;
}

static TrackMinMax *GenerateMinMax(const VectCollection &input) {
  TrackMinMax *extremes = new TrackMinMax;
  extremes->max = input[0];
  extremes->min = input[0];

  for (auto &i : input) {
    extremes->max._data = _mm_max_ps(i._data, extremes->max._data);
    extremes->min._data = _mm_min_ps(i._data, extremes->min._data);
  }
  /*for (int e = 0; e < 4; e++) {
    if (i[e] > extremes->min[e])
      extremes->min[e] = i[e];

    if (i[e] < extremes->max[e])
      extremes->max[e] = i[e];
  }*/

  extremes->min -= extremes->max;

  return extremes;
}

static TrackMinMax *EncodeWithinAABB(const VectCollection &input,
                                     VectCollection &output) {
  TrackMinMax *extremes = GenerateMinMax(input);
  const size_t numValues = input.size();

  output.resize(numValues);

  for (uint32 t = 0; t < 4; t++)
    if (!extremes->min[t])
      extremes->min[t] = 1.0f;

  for (size_t i = 0; i < numValues; i++)
    output[i] = (input[i] - extremes->max) / extremes->min;

  return extremes;
}

static TrackMinMax *Encode2ComponentQuat(const VectCollection &input,
                                         TrackTypesShared trackType,
                                         VectCollection &values) {
  TrackMinMax *extremes = GenerateMinMax(input);
  const size_t numValues = input.size();
  uint32 fakeComponent = 0;

  values.resize(numValues);

  if (trackType == TrackTypesShared::BiLinearRotationQuatYW_14bit)
    fakeComponent = 1;
  else if (trackType == TrackTypesShared::BiLinearRotationQuatZW_14bit)
    fakeComponent = 2;

  const uint32 comps[2] = {fakeComponent, 3};

  for (size_t i = 0; i < numValues; i++)
    for (const uint32 cComp : comps)
      values[i][cComp] =
          (input[i][cComp] - extremes->max[cComp]) / extremes->min[cComp];

  return extremes;
}

// version 2+ only
/*
  3 stages for vector 3 curves

  LinearVector3
  BiLinearVector3_16bit
  BiLinearVector3_8bit

  stages for quat compressions

  BiLinearRotationQuatXW_14bit
  BiLinearRotationQuatYW_14bit
  BiLinearRotationQuatZW_14bit
  LinearRotationQuat4_14bit -- not sure, where to put this one, yet
  BiLinearRotationQuat4_11bit
  BiLinearRotationQuat4_9bit
  BiLinearRotationQuat4_7bit

*/

static Vector4A16 floors(const Vector4A16 &input) {
  return Vector4A16(IVector4A16(input.Convert<int32>()));
}

static VectCollection QuantizeVectors(float fractal,
                                      const VectCollection &input) {
  VectCollection output;

  output.reserve(input.size());
  fractal = 1.0f / fractal;

  for (auto &v : input) {
    const Vector4A16 cVec = v * fractal;
    const Vector4A16 baseNum = floors(cVec);
    const Vector4A16 frac = cVec - baseNum;
    const Vector4A16 cmpRes(_mm_cmpgt_ps(frac._data, Vector4A16(0.5f)._data));
    const Vector4A16 approxVec = baseNum + (Vector4A16(1.0f) - frac) + 1.0f;

    output.push_back((cVec & ~cmpRes) | (approxVec & cmpRes));
  }

  return output;
}

struct CompressionReport {
  float maxFrac, avgFrac, maxDiff, avgDiff;
};

static CompressionReport ReportCompression(const float fractal,
                                           const VectCollection &input,
                                           const VectCollection &oldData) {
  const float *rawData = reinterpret_cast<const float *>(input.data());
  const float *oldRawData = reinterpret_cast<const float *>(oldData.data());
  const size_t numRawValues = input.size() * 4;
  CompressionReport result = {};

  for (size_t i = 0; i < numRawValues; i++) {
    const float &rawElement = rawData[i];
    const float fractalLess = std::floor(rawElement);
    float baseLess = rawElement - fractalLess;

    result.avgFrac += baseLess;
    result.maxFrac = std::max(baseLess, result.maxFrac);

    const float oldNewDiff = abs((fractalLess * fractal) - oldRawData[i]);

    result.avgDiff += oldNewDiff;
    result.maxDiff = std::max(oldNewDiff, result.maxDiff);
  }

  result.avgDiff /= static_cast<float>(numRawValues);
  result.avgFrac /= static_cast<float>(numRawValues);

  return result;
}

#include "datas/masterprinter.hpp"
#include <iomanip>

static const TrackTypesShared possibleVec3Types[] = {
    TrackTypesShared::BiLinearVector3_8bit,
    TrackTypesShared::BiLinearVector3_16bit, TrackTypesShared::LinearVector3};

static void ChooseRightVector3(const VectCollection &input) {
  static const float possibleFractals[] = {
      LMTTrackController::GetTrackMaxFrac(possibleVec3Types[0]),
      LMTTrackController::GetTrackMaxFrac(possibleVec3Types[1]),
      LMTTrackController::GetTrackMaxFrac(possibleVec3Types[2])};

  VectCollection values;
  std::unique_ptr<TrackMinMax> aabb(EncodeWithinAABB(input, values));
  const size_t numValues = input.size();

  VectCollection compressed = QuantizeVectors(possibleFractals[0], values);

  CompressionReport report =
      ReportCompression(possibleFractals[0], compressed, values);

  compressed = QuantizeVectors(possibleFractals[1], values);

  CompressionReport report2 =
      ReportCompression(possibleFractals[1], compressed, values);

  printer << std::setprecision(25) << report.avgFrac << ' ' << report2.avgFrac << ' ' << report.avgDiff << ' ' << report2.avgDiff>>
      1;

  int df = 0;
}

LMTOutputTracks CreateTrackFromRaw(const LMTRawTrack &rawData) {
  VectCollection positions, rotations, scales;
  const size_t numItems = rawData.data.size();

  positions.resize(numItems);
  rotations.resize(numItems);
  scales.resize(numItems);

  for (size_t i = 0; i < numItems; i++)
    rawData.data[i].Decompose(positions[i], rotations[i], scales[i]);

  ChooseRightVector3(positions);

  int res = 0;

  return {};
}