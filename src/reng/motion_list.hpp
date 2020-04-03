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
#include "asset.hpp"
#include "motion.hpp"
#include "uni_list_vector.hpp"
#include "uni_skeleton.hpp"

struct REMotlist : public REAssetBase {
  uint64 pad;
  REPointerX64<REPointerX64<REMotion>> motions;
  REPointerX64<char> unkOffset00;
  REPointerX64<char16_t> fileName;
  REPointerX64<char> null;
  int numMotions;

  int Fixup();
};

class REMotionBoneWrap : public uni::Bone {
public:
  REMotionBone *bone;
  REMotionBoneWrap *parent = nullptr;

  esMatrix44 Transform() const override {
    esMatrix44 retval(bone->rotation);
    retval.r4 = Vector4A16(bone->position, 1.0f);
    return retval;
  }
  const Bone *Parent() const override { return parent; }
  size_t Index() const override { return bone->boneHash; }
  std::string Name() const override { // FIX THIS
    return esStringConvert<char>(
        reinterpret_cast<const wchar_t *>(bone->boneName.operator->()));
  }
};

class RESkeletonWrap : public uni::Skeleton {
  uni::VectorList<uni::Bone, REMotionBoneWrap> bones;

public:
  void Assign(REMotionBone *data, size_t numBones);
  std::string Name() const override { return ""; }
  const uni::Bones &Bones() const override { return bones; }
};

typedef uni::VectorList<uni::Motion, REMotionAsset> MotionList;
typedef uni::VectorList<uni::Skeleton, RESkeletonWrap> SkeletonList;

class REMotlistAsset : public REAsset_internal,
                       public MotionList,
                       public SkeletonList {
  REMotlist &Get() { return REAssetBase::Get<REMotlist>(this->buffer); }
  const REMotlist &Get() const {
    return REAssetBase::Get<const REMotlist>(this->buffer);
  }

  int Fixup() override;
  void Build() override;

public:
  static const uint64 ID = CompileFourCC("mlst");
  static const uint64 VERSION = 85;
};