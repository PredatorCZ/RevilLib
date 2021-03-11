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
#include "motion_65.hpp"
#include "motion_list_60.hpp"

struct REMotlist85 : public REAssetBase {
  uint64 pad;
  esPointerX64<esPointerX64<REMotion65>> motions;
  esPointerX64<char> unkOffset00;
  esPointerX64<char16_t> fileName;
  esPointerX64<char> null;
  uint32 numMotions;

  void Fixup();
};

typedef uni::VectorList<uni::Motion, REMotion65Asset> MotionList85;

class REMotlist85Asset : public REAssetImpl,
                         public MotionList85,
                         public SkeletonList {
  REMotlist85 &Get() { return REAssetBase::Get<REMotlist85>(this->buffer); }
  const REMotlist85 &Get() const {
    return REAssetBase::Get<const REMotlist85>(this->buffer);
  }

  uni::BaseElementConst AsSkeletons() const override {
    return {static_cast<const SkeletonList *>(this), false};
  }

  uni::BaseElementConst AsMotions() const override {
    return {static_cast<const MotionList85 *>(this), false};
  }

  void Fixup() override;
  void Build() override;

public:
  static constexpr uint64 ID = CompileFourCC("mlst");
  static constexpr uint64 VERSION = 85;
};
