/*  Revil Format Library
    Copyright(C) 2017-2026 Lukas Cone

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
#include "spike/type/matrix44.hpp"

struct MtVector4 : Vector4A16 {};
struct MtVector3 : Vector4A16 {};

struct MtAABB {
  MtVector3 min;
  MtVector3 max;
};

struct MtOBB {
  es::Matrix44 transform;
  MtVector3 extents;
};

struct MtFloat3 {
  float x;
  float y;
  float z;
};
