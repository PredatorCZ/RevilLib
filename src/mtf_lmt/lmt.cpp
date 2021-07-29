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

#include "datas/reflector.hpp"
#include "internal.hpp"

REFLECT(CLASS(TrackMinMax), MEMBER(min), MEMBER(max));

LMT::LMT() : pi(std::make_unique<LMTImpl>()) {}
LMT::LMT(LMT &&) = default;
LMT::~LMT() = default;

LMTVersion LMT::Version() const { return pi->props.version; }
LMTArchType LMT::Architecture() const { return pi->props.arch; }
auto LMT::CreateAnimation() const { return LMTAnimation::Create(pi->props); }

LMT::operator uni::MotionsConst() const { return uni::MotionsConst{pi.get()}; }

void LMT::Version(LMTVersion _version, LMTArchType _arch) {
  if (!pi->masterBuffer.empty()) {
    throw std::runtime_error("Cannot set version for read only class!");
  }

  if (!pi->storage.empty()) {
    throw std::runtime_error("Cannot set version for already used class.");
  }

  pi->props.version = _version;
  pi->props.arch = _arch;
}

void LMT::AppendAnimation(LMTAnimation *ani) {
  if (ani && *ani != pi->props) {
    throw std::runtime_error("Cannot append animation. Properties mismatch.");
  }

  pi->storage.emplace_back(ani, false);
}

LMTAnimation *LMT::AppendAnimation() {
  pi->storage.emplace_back(uni::ToElement(CreateAnimation()));
  return pi->storage.back().get();
}

void LMT::InsertAnimation(LMTAnimation *ani, size_t at, bool replace) {
  if (*ani != pi->props) {
    throw std::runtime_error("Cannot append animation. Properties mismatch.");
  }

  if (at >= pi->storage.size()) {
    pi->storage.resize(at);
    pi->storage.emplace_back(ani);
  } else {
    pi->storage[at] = LMTImpl::class_type(ani, false);
  }
}
