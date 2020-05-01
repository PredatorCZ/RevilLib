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
#include "uni/list_vector.hpp"
#include "uni/skeleton.hpp"
#include "datas/unicode.hpp"
#include "motion_43.hpp"

struct REMotlist60 : public REAssetBase {
  uint64 pad;
  esPointerX64<esPointerX64<REMotion43>> motions;
  esPointerX64<char> unkOffset00;
  esPointerX64<char16_t> fileName;
  uint32 numMotions;

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
  std::string Name() const override {
    return es::ToUTF8(bone->boneName.operator->());
  }
};

class RESkeletonWrap : public uni::Skeleton {
  uni::VectorList<uni::Bone, REMotionBoneWrap> bones;

public:
  void Assign(REMotionBone *data, size_t numBones);
  std::string Name() const override { return ""; }
  uni::SkeletonBonesConst Bones() const override {
    return uni::SkeletonBonesConst(&bones, false);
  }
};

typedef uni::VectorList<uni::Motion, REMotion43Asset> MotionList60;
typedef uni::VectorList<uni::Skeleton, RESkeletonWrap> SkeletonList;

class REMotlist60Asset : public REAsset_internal,
                       public MotionList60,
                       public SkeletonList {
  REMotlist60 &Get() { return REAssetBase::Get<REMotlist60>(this->buffer); }
  const REMotlist60 &Get() const {
    return REAssetBase::Get<const REMotlist60>(this->buffer);
  }

  int Fixup() override;
  void Build() override;

public:
  static const uint64 ID = CompileFourCC("mlst");
  static const uint64 VERSION = 60;
};