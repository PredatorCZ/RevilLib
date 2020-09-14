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

#include "motion_list_99.hpp"
#include "motion_78.hpp"

int REMotlist99::Fixup() {
  char *masterBuffer = reinterpret_cast<char *>(this);

  motions.Fixup(masterBuffer);
  unkOffset00.Fixup(masterBuffer);
  fileName.Fixup(masterBuffer);
  null.Fixup(masterBuffer);

  for (uint32 m = 0; m < numMotions; m++) {
    motions[m].Fixup(masterBuffer);

    REAssetBase *cMotBase = motions[m];

    if (!cMotBase || cMotBase->assetID != REMotion78Asset::VERSION ||
        cMotBase->assetFourCC != REMotion78Asset::ID) {
      continue;
    }

    REMotion78 *cMot = static_cast<REMotion78 *>(cMotBase);
    cMot->Fixup();

    char *localBuffer = reinterpret_cast<char *>(cMot);

    if (cMot->pad || !cMot->bones) {
      continue;
    }

    cMot->bones.Fixup(localBuffer);
    cMot->bones->ptr.Fixup(localBuffer);
    REMotionBone *bonesPtr = cMot->bones->ptr;

    if (!bonesPtr) {
      continue;
    }

    for (size_t b = 0; b < cMot->numBones; b++) {
      bonesPtr[b].Fixup(localBuffer);
    }
  }

  return 0;
}

void REMotlist99Asset::Build() {
  REMotlist99 &data = Get();
  const size_t numAnims = data.numMotions;

  auto &motionListStorage = static_cast<MotionList99 &>(*this).storage;
  auto &skeletonStorage = static_cast<SkeletonList &>(*this).storage;

  for (size_t m = 0; m < numAnims; m++) {
    auto cMot = data.motions[m];

    if (!cMot || cMot->assetID != REMotion78Asset::VERSION ||
        cMot->assetFourCC != REMotion78Asset::ID) {
      continue;
    }

    motionListStorage.emplace_back();
    std::prev(motionListStorage.end())->Assign(cMot);

    if (cMot->pad || !cMot->bones || !cMot->bones->ptr) {
      continue;
    }

    skeletonStorage.emplace_back();
    std::prev(skeletonStorage.end())->Assign(cMot->bones->ptr, cMot->numBones);
  }
}

int REMotlist99Asset::Fixup() {
  REMotlist99 &data = Get();
  int rtVal = data.Fixup();
  Build();

  return rtVal;
}
