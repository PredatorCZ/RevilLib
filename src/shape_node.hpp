/*  Revil Format Library
    Copyright(C) 2026 Lukas Cone

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
#include "property.hpp"

struct ShapeNodeV1 {
  uint8 mShape;
  uint8 mJnt0;
  uint8 mJnt1;
  uint32 pad_;
  uint64a4 mAttr64;
  float mRadius;
  float mX0;
  float mY0;
  float mZ0;
  float mX1;
  float mY1;
  float mZ1;
  int32 mHandle;
};

struct ShapeNodeV2 {
  uint8 mShape;
  uint8 mPosType;
  uint8 mJnt0;
  uint8 mJnt1;
  uint32 pad_;
  uint64a4 mAttr64[2];
  float mRadius;
  float mX0;
  float mY0;
  float mZ0;
  float mX1;
  float mY1;
  float mZ1;
  float mAX;
  float mAY;
  float mAZ;
  float mRX;
  float mRY;
  float mRZ;
  int32 mHandle;
};

inline void ToXML(const ShapeNodeV1 &data, pugi::xml_node node) {
  NewPrimitive(node, "mShape", data.mShape);
  NewPrimitive(node, "mJnt0", data.mJnt0);
  NewPrimitive(node, "mJnt1", data.mJnt1);
  NewPrimitive(node, "mAttr64", data.mAttr64);
  NewPrimitive(node, "mRadius", data.mRadius);
  NewPrimitive(node, "mX0", data.mX0);
  NewPrimitive(node, "mY0", data.mY0);
  NewPrimitive(node, "mZ0", data.mZ0);
  NewPrimitive(node, "mX1", data.mX1);
  NewPrimitive(node, "mY1", data.mY1);
  NewPrimitive(node, "mZ1", data.mZ1);
  NewPrimitive(node, "mHandle", data.mHandle);
}

inline void ToXML(const ShapeNodeV2 &data, pugi::xml_node node) {
  NewPrimitive(node, "mShape", data.mShape);
  NewPrimitive(node, "mPosType", data.mPosType);
  NewPrimitive(node, "mJnt0", data.mJnt0);
  NewPrimitive(node, "mJnt1", data.mJnt1);
  NewPrimitive(node, "mAttr64[0]", data.mAttr64[0]);
  NewPrimitive(node, "mAttr64[1]", data.mAttr64[1]);
  NewPrimitive(node, "mRadius", data.mRadius);
  NewPrimitive(node, "mX0", data.mX0);
  NewPrimitive(node, "mY0", data.mY0);
  NewPrimitive(node, "mZ0", data.mZ0);
  NewPrimitive(node, "mX1", data.mX1);
  NewPrimitive(node, "mY1", data.mY1);
  NewPrimitive(node, "mZ1", data.mZ1);
  NewPrimitive(node, "mAX", data.mAX);
  NewPrimitive(node, "mAY", data.mAY);
  NewPrimitive(node, "mAZ", data.mAZ);
  NewPrimitive(node, "mRX", data.mRX);
  NewPrimitive(node, "mRY", data.mRY);
  NewPrimitive(node, "mRZ", data.mRZ);
  NewPrimitive(node, "mHandle", data.mHandle);
}
