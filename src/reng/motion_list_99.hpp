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

#pragma once
#include "motion_78.hpp"
#include "motion_list_85.hpp"

class REMotlist99 : public REMotlist85 {
public:
};

typedef uni::VectorList<uni::Motion, REMotion78Asset> MotionList99;

class REMotlist99Asset : public REAssetImpl,
                         public MotionList99,
                         public SkeletonList {
  REMotlist99 &Get() { return REAssetBase::Get<REMotlist99>(this->buffer); }
  const REMotlist99 &Get() const {
    return REAssetBase::Get<const REMotlist99>(this->buffer);
  }

  uni::BaseElementConst AsSkeletons() const override {
    return {static_cast<const SkeletonList *>(this), false};
  }

  uni::BaseElementConst AsMotions() const override {
    return {static_cast<const MotionList99 *>(this), false};
  }

  void Fixup(std::vector<void *> &ptrStore) override;
  void Build() override;

public:
  static constexpr uint64 ID = CompileFourCC("mlst");
  static constexpr uint64 VERSION = 99;
};
