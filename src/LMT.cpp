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

#include "LMTInternal.h"
#include "datas/masterprinter.hpp"

REFLECTOR_CREATE(TrackMinMax, 1, VARNAMES, min, max);

void LMT::Version(V _version, Architecture arch) {
  if (!masterBuffer.empty()) {
    throw std::runtime_error("Cannot set version for read only class!");
  }

  if (!storage.empty()) {
    throw std::runtime_error("Cannot set version for already used class.");
  }

  props.version = static_cast<uint8>(_version);
  props.ptrSize = arch == X64 ? 8 : 4;
}

void LMT::AppendAnimation(LMTAnimation *ani) {
  if (ani && *ani != props) {
    printerror("[LMT] Cannot append animation. Properties mismatch.");
    return;
  }

  storage.push_back(pointer_class_type(ani, false));
}

LMTAnimation *LMT::AppendAnimation() {
  LMTAnimation *cAni = CreateAnimation();
  storage.emplace_back(cAni);
  return cAni;
}

void LMT::InsertAnimation(LMTAnimation *ani, uint32 at, bool replace) {
  if (*ani != props) {
    printerror("[LMT] Cannot append animation. Properties mismatch.");
    return;
  }

  if (at >= storage.size()) {
    storage.resize(at);
    storage.push_back(pointer_class_type(ani));
  } else {
    storage[at] = pointer_class_type(ani, false);
  }
}

LMTAnimation *LMT::CreateAnimation() const {
  return LMTAnimation::Create(props);
}

uni::MotionTrack::TrackType_e LMTTrack::TrackType() const {
  const auto iType = this->GetTrackType();

  switch (iType) {
  case TrackType_AbsolutePosition:
  case TrackType_LocalPosition:
    return MotionTrack::TrackType_e::Position;

  case TrackType_AbsoluteRotation:
  case TrackType_LocalRotation:
    return MotionTrack::TrackType_e::Position;

  default:
    return MotionTrack::TrackType_e::Scale;
  }
}

void LMTTrack::GetValue(uni::RTSValue &output, float time) const {
  throw std::logic_error("Unsupported call!");
}
void LMTTrack::GetValue(esMatrix44 &output, float time) const {
  throw std::logic_error("Unsupported call!");
}
void LMTTrack::GetValue(float &output, float time) const {
  throw std::logic_error("Unsupported call!");
}
