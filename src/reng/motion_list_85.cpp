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

#include "motion_list_85.hpp"

void REMotlist85::Fixup() {
  char *masterBuffer = reinterpret_cast<char *>(this);

  motions.Fixup(masterBuffer);
  unkOffset00.Fixup(masterBuffer);
  fileName.Fixup(masterBuffer);
  null.Fixup(masterBuffer);

  for (uint32 m = 0; m < numMotions; m++) {
    motions[m].Fixup(masterBuffer);

    REAssetBase *cMotBase = motions[m];

    if (!cMotBase || cMotBase->assetID != REMotion65Asset::VERSION ||
        cMotBase->assetFourCC != REMotion65Asset::ID) {
      continue;
    }

    motions[m]->Fixup();
  }
}

void REMotlist85Asset::Build() {
  REMotlist85 &data = Get();
  const size_t numAnims = data.numMotions;

  auto &motionListStorage = static_cast<MotionList85 &>(*this).storage;

  for (size_t m = 0; m < numAnims; m++) {
    REAssetBase *cMot = data.motions[m];

    if (!cMot || cMot->assetID != REMotion65Asset::VERSION ||
        cMot->assetFourCC != REMotion65Asset::ID) {
      continue;
    }

    motionListStorage.emplace_back();
    motionListStorage.back().Assign(cMot);
  }

  auto &skeletonStorage = static_cast<SkeletonList &>(*this).storage;

  for (size_t m = 0; m < numAnims; m++) {
    auto cMot = data.motions[m];

    if (!cMot || cMot->assetID != REMotion65Asset::VERSION ||
        cMot->assetFourCC != REMotion65Asset::ID || !cMot->bones) {
      continue;
    }

    skeletonStorage.emplace_back();
    skeletonStorage.back().Assign(cMot->bones->ptr, cMot->numBones);
  }
}

void REMotlist85Asset::Fixup() {
  REMotlist85 &data = Get();
  data.Fixup();
  Build();
}
