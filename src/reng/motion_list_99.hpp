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
#include "motion_list.hpp"

class REMotlist99 : public REMotlist {
public:
  int Fixup();
};

typedef uni::VectorList<uni::Motion, REMotion78Asset> MotionList99;

class REMotlist99Asset : public REAsset_internal,
                         public MotionList99,
                         public SkeletonList {
  REMotlist99 &Get() { return REAssetBase::Get<REMotlist99>(this->buffer); }
  const REMotlist99 &Get() const {
    return REAssetBase::Get<const REMotlist99>(this->buffer);
  }

  int Fixup() override;
  void Build() override;

public:
  static const uint64 ID = CompileFourCC("mlst");
  static const uint64 VERSION = 99;
};
