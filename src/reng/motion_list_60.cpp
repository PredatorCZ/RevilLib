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

template <> void ProcessClass(REMotlist60 &item, ProcessFlags flags) {
  flags.base = reinterpret_cast<char *>(&item);

  if (!es::FixupPointers(flags.base, *flags.ptrStore, item.motions,
                         item.unkOffset00, item.fileName)) {
    return;
  }

  auto motions = item.motions.operator->();

  for (uint32 m = 0; m < item.numMotions; m++) {
    motions[m].Fixup(flags.base, *flags.ptrStore);

    REAssetBase *cMotBase = motions[m];

    if (!cMotBase || cMotBase->assetID != REMotion43Asset::VERSION ||
        cMotBase->assetFourCC != REMotion43Asset::ID) {
      continue;
    }

    auto nFlags = flags;
    nFlags.base = reinterpret_cast<char *>(cMotBase);

    ProcessClass(*motions[m], nFlags);
  }
}

void REMotlist60Asset::Build() {
  REMotlist60 &data = Get();
  const size_t numAnims = data.numMotions;

  auto &motionListStorage = static_cast<MotionList60 &>(*this).storage;
  auto motions = data.motions.operator->();

  for (size_t m = 0; m < numAnims; m++) {
    REAssetBase *cMot = motions[m];

    if (!cMot || cMot->assetID != REMotion43Asset::VERSION ||
        cMot->assetFourCC != REMotion43Asset::ID) {
      continue;
    }

    motionListStorage.emplace_back(cMot);
  }

  auto &skeletonStorage = static_cast<SkeletonList &>(*this).storage;

  for (size_t m = 0; m < numAnims; m++) {
    auto cMot = motions[m];

    if (!cMot || cMot->assetID != REMotion43Asset::VERSION ||
        cMot->assetFourCC != REMotion43Asset::ID || !cMot->bones) {
      continue;
    }

    skeletonStorage.emplace_back();
    skeletonStorage.back().Assign(cMot->bones->ptr, cMot->numBones);
  }
}

void REMotlist60Asset::Fixup(std::vector<void *> &ptrStore) {
  ProcessFlags flags;
  flags.ptrStore = &ptrStore;
  ProcessClass(Get(), flags);
  Build();
}

void RESkeletonWrap::Assign(REMotionBone *data, size_t numBones) {
  for (size_t b = 0; b < numBones; b++) {
    bones.storage.emplace_back();
    bones.storage.back().bone = data + b;
  }

  for (size_t b = 0; b < numBones; b++) {
    char16_t **parentNameRaw = data[b].parentBoneNamePtr;

    if (!parentNameRaw) {
      continue;
    }

    std::u16string parentName = *parentNameRaw;

    for (auto &_b : bones.storage) {
      if (parentName == &*_b.bone->boneName) {
        bones.storage[b].parent = &_b;
        break;
      }
    }
  }
}

namespace revil {
template <> ES_EXPORT uni::SkeletonsConst REAsset::As<>() const {
  auto val = i->AsSkeletons();
  return {static_cast<typename uni::SkeletonsConst::pointer>(val.release()),
          false};
}
} // namespace revil
