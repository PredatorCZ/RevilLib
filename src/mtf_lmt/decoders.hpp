#include "revil/mot.hpp"
#include "spike/type/matrix44.hpp"

struct LMTRawTrack {
  typedef std::vector<es::Matrix44> InputRaw;

  const InputRaw &data;
  int32 boneID;
  uint8 boneType;
  float weight;

  LMTRawTrack(const InputRaw &inputData, int boneIndex)
      : data(inputData), boneID(boneIndex), boneType(0), weight(1.0f) {}
};

struct LMTOutputTracks {
  revil::LMTTrack *position, *rotation, *scale;
};

LMTOutputTracks CreateTrackFromRaw(const LMTRawTrack &rawData);
