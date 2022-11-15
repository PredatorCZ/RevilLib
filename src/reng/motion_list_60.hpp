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
#include "datas/unicode.hpp"
#include "motion_43.hpp"
#include "uni/list_vector.hpp"
#include "uni/skeleton.hpp"
#include "uni/rts.hpp"

struct REMotlist60 : public REAssetBase {
  uint64 pad;
  esPointerX64<esPointerX64<REMotion43>> motions;
  esPointerX64<char> unkOffset00;
  esPointerX64<char16_t> fileName;
  uint32 numMotions;
};

class REMotionBoneWrap : public uni::Bone {
public:
  REMotionBone *bone;
  REMotionBoneWrap *parent = nullptr;

  uni::TransformType TMType() const override { return uni::TMTYPE_RTS; }

  void GetTM(uni::RTSValue &out) const override {
    out.rotation = bone->rotation;
    out.translation = bone->position;
  }

  const Bone *Parent() const override { return parent; }
  size_t Index() const override { return bone->boneHash; }
  std::string Name() const override {
    return es::ToUTF8(bone->boneName.operator->());
  }

  operator uni::Element<const uni::Bone>() const {
    return uni::Element<const uni::Bone>{this, false};
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

  operator uni::Element<const uni::Skeleton>() const {
    return uni::Element<const uni::Skeleton>{this, false};
  }
};

typedef uni::VectorList<uni::Motion, REMotion43Asset> MotionList60;
typedef uni::VectorList<uni::Skeleton, RESkeletonWrap> SkeletonList;

class REMotlist60Asset : public REAssetImpl,
                         public MotionList60,
                         public SkeletonList {
  REMotlist60 &Get() { return REAssetBase::Get<REMotlist60>(this->buffer); }
  const REMotlist60 &Get() const {
    return REAssetBase::Get<const REMotlist60>(this->buffer);
  }

  uni::BaseElementConst AsSkeletons() const override {
    return {static_cast<const SkeletonList *>(this), false};
  }

  uni::BaseElementConst AsMotions() const override {
    return {static_cast<const MotionList60 *>(this), false};
  }

  void Fixup(std::vector<void *> &ptrStore) override;
  void Build() override;

public:
  static constexpr uint64 ID = CompileFourCC("mlst");
  static constexpr uint64 VERSION = 60;
};
