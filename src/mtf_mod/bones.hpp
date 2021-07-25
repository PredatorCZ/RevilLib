/*  Revil Format Library
    Copyright(C) 2017-2021 Lukas Cone

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
#include "datas/vectors.hpp"
#include "uni/skeleton.hpp"

struct MODBoneV1 {
  uint8 index;
  uint8 parentIndex;
  uint32 null;
  float parentDistance;
  Vector absolutePosition;

  operator MODBone() const {
    return {index, parentIndex, 0xffff, -1.f, parentDistance, absolutePosition};
  }
};

struct MODBoneV1_5 {
  uint8 index;
  uint8 parentIndex;
  uint8 mirrorIndex;
  float furthestVertexDistance; // furthest distance to vertex influenced by
                                // this bone
  float parentDistance;
  Vector absolutePosition;

  operator MODBone() const {
    return {index,          parentIndex,
            mirrorIndex,    furthestVertexDistance,
            parentDistance, absolutePosition};
  }
};

struct MODBoneV2 {
  uint16 index;
  uint8 parentIndex;
  uint8 mirrorIndex;
  float furthestVertexDistance; // furthest distance to vertex influenced by
                                // this bone
  float parentDistance;
  Vector absolutePosition;

  operator MODBone() const {
    return {index,          parentIndex,
            mirrorIndex,    furthestVertexDistance,
            parentDistance, absolutePosition};
  }
};
