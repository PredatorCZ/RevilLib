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

#include "motion_list_486.hpp"

void REMotlist486::Fixup() {
  char *masterBuffer = reinterpret_cast<char *>(this);
  if (!es::FixupPointers(masterBuffer, ptrStore, motions, unkOffset00, fileName,
                         null)) {
    return;
  }

  for (uint32 m = 0; m < numMotions; m++) {
    motions[m].Fixup(masterBuffer, &ptrStore);

    REAssetBase *cMotBase = motions[m];

    if (!cMotBase || cMotBase->assetID != REMotion458Asset::VERSION ||
        cMotBase->assetFourCC != REMotion458Asset::ID) {
      continue;
    }

    REMotion458 *cMot = static_cast<REMotion458 *>(cMotBase);
    cMot->Fixup();

    char *localBuffer = reinterpret_cast<char *>(cMot);

    if (cMot->pad || !cMot->bones) {
      continue;
    }

    cMot->bones.Fixup(localBuffer, &ptrStore);
    cMot->bones->ptr.Fixup(localBuffer, &ptrStore);
    REMotionBone *bonesPtr = cMot->bones->ptr;

    if (!bonesPtr) {
      continue;
    }

    for (size_t b = 0; b < cMot->numBones; b++) {
      bonesPtr[b].Fixup(localBuffer);
    }
  }
}

void REMotlist486Asset::Build() {
  REMotlist486 &data = Get();
  const size_t numAnims = data.numMotions;

  auto &motionListStorage = static_cast<MotionList486 &>(*this).storage;
  auto &skeletonStorage = static_cast<SkeletonList &>(*this).storage;

  for (size_t m = 0; m < numAnims; m++) {
    auto cMot = data.motions[m];

    if (!cMot || cMot->assetID != REMotion458Asset::VERSION ||
        cMot->assetFourCC != REMotion458Asset::ID) {
      continue;
    }

    motionListStorage.emplace_back<REAssetBase *>(cMot);

    if (cMot->pad || !cMot->bones || !cMot->bones->ptr) {
      continue;
    }

    skeletonStorage.emplace_back();
    skeletonStorage.back().Assign(cMot->bones->ptr, cMot->numBones);
  }
}

void REMotlist486Asset::Fixup() {
  REMotlist486 &data = Get();
  data.Fixup();
  Build();
}
