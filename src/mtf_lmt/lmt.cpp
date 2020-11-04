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

#include "internal.hpp"

REFLECTOR_CREATE(TrackMinMax, 1, VARNAMES, min, max);

void LMT::Version(LMTVersion _version, LMTArchType _arch) {
  if (!masterBuffer.empty()) {
    throw std::runtime_error("Cannot set version for read only class!");
  }

  if (!storage.empty()) {
    throw std::runtime_error("Cannot set version for already used class.");
  }

  props.version = _version;
  props.arch = _arch;
}

void LMT::AppendAnimation(LMTAnimation *ani) {
  if (ani && *ani != props) {
    throw std::runtime_error("Cannot append animation. Properties mismatch.");
  }

  storage.emplace_back(ani, false);
}

LMTAnimation *LMT::AppendAnimation() {
  LMTAnimation *cAni = CreateAnimation();
  storage.emplace_back(cAni);
  return cAni;
}

void LMT::InsertAnimation(LMTAnimation *ani, uint32 at, bool replace) {
  if (*ani != props) {
    std::runtime_error("Cannot append animation. Properties mismatch.");
  }

  if (at >= storage.size()) {
    storage.resize(at);
    storage.emplace_back(ani);
  } else {
    storage[at] = class_type(ani, false);
  }
}

LMTAnimation *LMT::CreateAnimation() const {
  return LMTAnimation::Create(props);
}
