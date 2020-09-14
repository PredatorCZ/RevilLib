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

#include "motion_list_60.hpp"

int REMotlist60::Fixup() {
  char *masterBuffer = reinterpret_cast<char *>(this);

  motions.Fixup(masterBuffer);
  unkOffset00.Fixup(masterBuffer);
  fileName.Fixup(masterBuffer);

  for (uint32 m = 0; m < numMotions; m++) {
    motions[m].Fixup(masterBuffer);

    REAssetBase *cMotBase = motions[m];

    if (!cMotBase || cMotBase->assetID != REMotion43Asset::VERSION ||
        cMotBase->assetFourCC != REMotion43Asset::ID) {
      continue;
    }

    motions[m]->Fixup();
  }

  return 0;
}

void REMotlist60Asset::Build() {
  REMotlist60 &data = Get();
  const size_t numAnims = data.numMotions;

  auto &motionListStorage = static_cast<MotionList60 &>(*this).storage;

  for (size_t m = 0; m < numAnims; m++) {
    REAssetBase *cMot = data.motions[m];

    if (!cMot || cMot->assetID != REMotion43Asset::VERSION ||
        cMot->assetFourCC != REMotion43Asset::ID) {
      continue;
    }

    motionListStorage.emplace_back();
    std::prev(motionListStorage.end())->Assign(cMot);
  }

  auto &skeletonStorage = static_cast<SkeletonList &>(*this).storage;

  for (size_t m = 0; m < numAnims; m++) {
    auto cMot = data.motions[m];

    if (!cMot || cMot->assetID != REMotion43Asset::VERSION ||
        cMot->assetFourCC != REMotion43Asset::ID || !cMot->bones) {
      continue;
    }

    skeletonStorage.emplace_back();
    std::prev(skeletonStorage.end())->Assign(cMot->bones->ptr, cMot->numBones);
  }
}

int REMotlist60Asset::Fixup() {
  REMotlist60 &data = Get();
  int rtVal = data.Fixup();
  Build();
  return rtVal;
}

void RESkeletonWrap::Assign(REMotionBone *data, size_t numBones) {
  for (size_t b = 0; b < numBones; b++) {
    bones.storage.emplace_back();
    std::prev(bones.storage.end())->bone = data + b;
  }

  for (size_t b = 0; b < numBones; b++) {
    char16_t **parentNameRaw = data[b].parentBoneNamePtr;

    if (!parentNameRaw)
      continue;

    std::u16string parentName = *parentNameRaw;

    for (auto &_b : bones.storage) {
      if (parentName == &*_b.bone->boneName) {
        bones.storage[b].parent = &_b;
        break;
      }
    }
  }
}
