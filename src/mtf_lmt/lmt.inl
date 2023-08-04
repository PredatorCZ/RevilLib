#pragma once
#include "spike/classgen.hpp"

enum LMTVersion : uint8 {
  LMT22 = 22, // DR
  LMT40 = 40, // LP
  LMT49 = 49, // DMC4
  LMT50 = 50, // LP PS3
  LMT51 = 51, // RE5
  LMT56 = 56, // LP2
  LMT57 = 57, // RE:M 3DS
  LMT66 = 66, // DD, DD:DA
  LMT67 = 67, // Other, generic MTF v2 format
  LMT92 = 92, // MH:W
  LMT95 = 95, // Iceborne
};

struct AnimEventsHeaderV2;
