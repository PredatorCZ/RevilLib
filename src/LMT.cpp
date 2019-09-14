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

#include "LMT.h"
#include "datas/masterprinter.hpp"
#include "datas/reflector.hpp"

#include "LMTInternal.h"

REFLECTOR_CREATE(TrackMinMax, 1, VARNAMES, min, max);

int LMT::Version(V _version, Architecture arch) {
  if (masterBuffer) {
    printerror("[LMT] Cannot set version for read only class.");
    return 1;
  }

  if (animations.size()) {
    printerror("[LMT] Cannot set version for already used class.");
    return 2;
  }

  props.version = static_cast<uchar>(_version);
  props.ptrSize = arch == X64 ? 8 : 4;

  return 0;
}

LMT::LMT() : masterBuffer(nullptr), props() {}

LMT::~LMT() {
  if (masterBuffer)
    free(masterBuffer);

  for (auto &a : animations)
    delete a;
}

void LMT::AppendAnimation(LMTAnimation *ani) {
  if (ani && *ani != props) {
    printerror("[LMT] Cannot append animation. Properties mismatch.");
    return;
  }

  animations.push_back(ani);
}

LMTAnimation *LMT::AppendAnimation() {
  LMTAnimation *cAni = CreateAnimation();
  animations.push_back(cAni);
  return cAni;
}

void LMT::InsertAnimation(LMTAnimation *ani, int at, bool replace) {
  if (*ani != props) {
    printerror("[LMT] Cannot append animation. Properties mismatch.");
    return;
  }

  if (at >= animations.size()) {
    animations.resize(at);
    animations.push_back(ani);
  } else if (replace) {
    if (animations[at])
      delete animations[at];

    animations[at] = ani;
  } else
    animations.insert(animations.begin() + at, ani);
}

LMTAnimation *LMT::CreateAnimation() const { 
  return LMTAnimation::Create(props);
}
