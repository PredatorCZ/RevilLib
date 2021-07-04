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
#include "datas/supercore.hpp"

struct MODHeaderCommon {
  uint32 id;
  int16 version;
  uint16 numBones;
  uint16 numMeshes;
  uint16 numMaterials;
  uint32 numVertices;
  uint32 numIndices;
  uint32 numEdges;
};

struct MODHeaderX99 : MODHeaderCommon {
  uint32 vertexBufferSize;
  uint32 unkBufferSize;
  uint32 numTextures;
  uint32 numGroups;
  uint32 numBoneMaps;
  uint32 bones;
  uint32 groups;
  uint32 textures;
  uint32 meshes;
  uint32 vertexBuffer;
  uint32 unkBuffer;
  uint32 indices;
};

struct MODHeaderXC5 : MODHeaderCommon {
  uint32 vertexBufferSize;
  uint32 numTextures;
  uint32 numGroups;
  uint32 bones;
  uint32 groups;
  uint32 textures;
  uint32 meshes;
  uint32 vertexBuffer;
  uint32 indices;
};

struct MODHeaderX70 : MODHeaderCommon {
  uint32 vertexBufferSize;
  uint32 unkBufferSize;
  uint32 numTextures;
  uint32 null;
  uint32 bones;
  uint32 textures;
  uint32 meshes;
  uint32 vertexBuffer;
  uint32 unkBuffer;
  uint32 indices;
};

struct MODHeaderX170 : MODHeaderCommon {
  uint32 vertexBufferSize;
  uint32 unkBufferSize;
  uint64 numTextures;
  uint64 bones;
  uint64 textures;
  uint64 meshes;
  uint64 vertexBuffer;
  uint64 unkBuffer;
  uint64 indices;
};
