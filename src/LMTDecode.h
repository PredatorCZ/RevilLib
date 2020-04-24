#include "LMT.h"
#include "datas/Matrix44.hpp"


struct LMTRawTrack {
  typedef std::vector<esMatrix44> InputRaw;
  
  const InputRaw &data;
  int32 boneID;
  uint8 boneType;
  float weight;

  LMTRawTrack(const InputRaw &inputData, int boneIndex)
      : data(inputData), boneID(boneIndex), boneType(0), weight(1.0f) {}
};

struct LMTOutputTracks
{
    LMTTrack *position, *rotation, *scale;
};

LMTOutputTracks CreateTrackFromRaw(const LMTRawTrack &rawData);