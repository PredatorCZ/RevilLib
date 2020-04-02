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

#include "motion_list.hpp"

int REMotlist::Fixup() {
  char *masterBuffer = reinterpret_cast<char *>(this);

  motions.Fixup(masterBuffer);
  unkOffset00.Fixup(masterBuffer);
  fileName.Fixup(masterBuffer);
  null.Fixup(masterBuffer);

  for (int m = 0; m < numMotions; m++) {
    motions[m].Fixup(masterBuffer);
    motions[m]->Fixup();
  }

  return 0;
}

void REMotlistAsset::Build() {
  REMotlist &data = Get();
  const size_t numAnims = data.numMotions;

  auto &motionListStorage = static_cast<MotionList &>(*this).storage;

  for (size_t m = 0; m < numAnims; m++) {
    motionListStorage.emplace_back(
        std::unique_ptr<REMotionAsset>(new REMotionAsset()));
    std::prev(motionListStorage.end())->get()->Assign(data.motions[m]);
  }

  auto &skeletonStorage = static_cast<SkeletonList &>(*this).storage;

  for (size_t m = 0; m < numAnims; m++) {
    auto &cMot = *data.motions[m];
    skeletonStorage.emplace_back(
        std::unique_ptr<RESkeletonWrap>(new RESkeletonWrap()));
    std::prev(skeletonStorage.end())
        ->get()
        ->Assign(cMot.bones->ptr, cMot.numBones);
  }
}

int REMotlistAsset::Fixup() {
  REMotlist &data = Get();
  int rtVal = data.Fixup();
  Build();
  return rtVal;
}

void RESkeletonWrap::Assign(REMotionBone *data, size_t numBones) {
  for (size_t b = 0; b < numBones; b++) {
    bones.storage.emplace_back(
        std::unique_ptr<REMotionBoneWrap>(new REMotionBoneWrap()));
    auto &cBone = *std::prev(bones.storage.end());
    cBone->bone = data + b;
  }

  for (size_t b = 0; b < numBones; b++) {
    char16_t **parentNameRaw = data[b].parentBoneNamePtr;

    if (!parentNameRaw)
      continue;

    std::u16string parentName = *parentNameRaw;

    for (auto &_b : bones.storage) {
      if (parentName == &*_b->bone->boneName) {
        bones.storage[b]->parent = _b.get();
        break;
      }
    }
  }
}
