#include <LMTCodecs.h>

typedef std::vector<Vector4> QuatCollection;
typedef std::vector<Vector> PointCollection;

static TrackTypesShared Choose2ComponentQuat(const QuatCollection &input) {
  bool staticComponent[4] = {true, true, true, false};
  int &staticComponentInt = reinterpret_cast<int &>(staticComponent);
  Vector4 firstItem = input[0];
  float componentEpsilon = 0.00001f;

  for (auto &i : input) {
    if (!staticComponentInt)
      return TrackTypesShared::None;

    for (int c = 0; c < 3; c++)
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

static TrackMinMax *GenerateMinMax(const QuatCollection &input) {
  TrackMinMax *extremes = new TrackMinMax;
  extremes->max = input[0];
  extremes->min = input[0];

  for (auto &i : input)
    for (int e = 0; e < 4; e++) {
      if (i[e] > extremes->min[e])
        extremes->min[e] = i[e];

      if (i[e] < extremes->max[e])
        extremes->max[e] = i[e];
    }

  extremes->min -= extremes->max;

  return extremes;
}

static TrackMinMax *Encode2ComponentQuat(const QuatCollection &input,
                                         TrackTypesShared trackType,
                                         QuatCollection &values) {
  TrackMinMax *extremes = GenerateMinMax(input);
  const size_t numValues = input.size();
  int fakeComponent = 0;

  values.resize(numValues);

  if (trackType == TrackTypesShared::BiLinearRotationQuatYW_14bit)
    fakeComponent = 1;
  else if (trackType == TrackTypesShared::BiLinearRotationQuatZW_14bit)
    fakeComponent = 2;

  const int comps[2] = {fakeComponent, 3};

  for (size_t i = 0; i < numValues; i++)
    for (int e = 0; e < 2; e++) {
      const int cComp = comps[e];
      values[i][cComp] =
          (input[i][cComp] - extremes->max[cComp]) / extremes->min[cComp];
    }

  return extremes;
}

