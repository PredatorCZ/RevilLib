/*  Revil Format Library
    Copyright(C) 2017-2023 Lukas Cone

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

#include "header.hpp"
#include "pugixml.hpp"
#include "revil/xfs.hpp"
#include "spike/io/binreader.hpp"
#include "spike/io/binwritter.hpp"
#include "spike/util/endian.hpp"
#include "traits.hpp"
#include <map>

using namespace revil;

static constexpr uint32 MODID = CompileFourCC("MOD");
static constexpr uint32 DOMID = CompileFourCC("\0DOM");
static constexpr uint32 RMDID = CompileFourCC("\0DMR");

template <class T> using use_name = decltype(std::declval<T>().Name());
template <class C>
constexpr static bool use_name_v = es::is_detected_v<use_name, C>;

template <class traits> void MODInner<traits>::Reflect(bool swap) {
  for (size_t i = 0; i < bones.size(); i++) {
    const bool isRoot =
        sizeof(bones[i].parentIndex) == 1 && bones[i].parentIndex == 0xff;
    MODBone bne{
        .index = bones[i].index,
        .parentIndex = uint16(isRoot ? 0xffff : bones[i].parentIndex),
    };
    this->simpleBones.emplace_back(bne);
  }

  this->primitives.reserve(meshes.size());
  if (swap) {
    for (size_t i = 0; i < meshes.size(); i++) {
      this->primitives.emplace_back(meshes[i].ReflectBE(*this));
    }
  } else {
    for (size_t i = 0; i < meshes.size(); i++) {
      this->primitives.emplace_back(meshes[i].ReflectLE(*this));
    }
  }

  for (auto &r : skinRemaps) {
    this->skins.emplace_back(r.bones, r.count);
  }

  for (auto &m : this->materials) {
    revil::MODMaterial &ref = this->materialRefs.emplace_back();
    if constexpr (use_name_v<std::decay_t<decltype(m.main)>>) {
      ref.name = m.main.Name();
    }

    ref.refl = &m.asMetadata;
  }
}

template <class material_type>
void MODMaterialProxy<material_type>::Write(BinWritterRef wr) const {
  auto main_ = main;
  main_.baseTextureIndex++;
  main_.normalTextureIndex++;
  main_.maskTextureIndex++;
  main_.lightTextureIndex++;
  main_.shadowTextureIndex++;
  main_.additionalTextureIndex++;
  main_.cubeMapTextureIndex++;
  main_.detailTextureIndex++;
  main_.AOTextureIndex++;
  wr.Write(main_);
}

template <class material_type>
void MODMaterialProxy<material_type>::Read(BinReaderRef_e rd) {
  rd.Read(main);
  if constexpr (!std::is_same_v<material_type, MODMaterialHash> &&
                !std::is_same_v<material_type, MODMaterialName>) {
    main.baseTextureIndex--;
    main.normalTextureIndex--;
    main.maskTextureIndex--;
    main.lightTextureIndex--;
    main.shadowTextureIndex--;
    main.additionalTextureIndex--;
    main.cubeMapTextureIndex--;
    main.detailTextureIndex--;
    main.AOTextureIndex--;
  }
}

#pragma region Endian Swappers

template <> void FByteswapper(MODBoneV1 &self, bool) {
  FByteswapper(self.absolutePosition);
  FByteswapper(self.parentDistance);
}

template <> void FByteswapper(MODBoneV1_5 &self, bool) {
  FByteswapper(self.absolutePosition);
  FByteswapper(self.parentDistance);
  FByteswapper(self.furthestVertexDistance);
}

template <> void FByteswapper(MODBoneV2 &self, bool) {
  FByteswapper(self.absolutePosition);
  FByteswapper(self.parentDistance);
  FByteswapper(self.furthestVertexDistance);
  FByteswapper(self.index);
}

template <> void FByteswapper(MODBounds &self, bool) {
  FByteswapper(self.bboxMax);
  FByteswapper(self.bboxMin);
  FByteswapper(self.boundingSphere);
}

template <> void FByteswapper(MODEnvelope &self, bool) {
  FByteswapper(self.absolutePosition);
  FByteswapper(self.boneIndex);
  FByteswapper(self.bounds);
}

template <> void FByteswapper(MODGroup &self, bool) {
  FByteswapper(self.boundingSphere);
  FByteswapper(self.index);
}

template <> void FByteswapper(MODMetaData &self, bool) {
  FByteswapper(self.lightGroup);
  FByteswapper(self.lowDistance);
  FByteswapper(self.middleDistance);
}

template <> void FByteswapper(MODMetaDataV2 &self, bool) {
  FByteswapper(static_cast<MODMetaData &>(self));
  FByteswapper(self.numEnvelopes);
}

template <> void FByteswapper(MODSkinRemap<24> &self, bool) {
  FByteswapper(self.count);
}

template <> void FByteswapper(MODSkinRemap<32> &self, bool) {
  FByteswapper(self.count);
}

template <> void FByteswapper(MODSkinRemap<64> &self, bool) {
  FByteswapper(self.count);
}

template <> void FByteswapper(MODHeaderCommon &self, bool) {
  FByteswapper(self.id);
  FByteswapper(self.version);
  FByteswapper(self.numBones);
  FByteswapper(self.numMeshes);
  FByteswapper(self.numMaterials);
  FByteswapper(self.numVertices);
  FByteswapper(self.numIndices);
  FByteswapper(self.numEdges);
}

template <> void FByteswapper(MODHeaderX99 &self, bool) {
  FByteswapper(static_cast<MODHeaderCommon &>(self));
  FByteswapper(self.vertexBufferSize);
  FByteswapper(self.unkBufferSize);
  FByteswapper(self.numTextures);
  FByteswapper(self.numGroups);
  FByteswapper(self.numBoneMaps);
  FByteswapper(self.bones);
  FByteswapper(self.groups);
  FByteswapper(self.textures);
  FByteswapper(self.meshes);
  FByteswapper(self.vertexBuffer);
  FByteswapper(self.unkBuffer);
  FByteswapper(self.indices);
}

template <> void FByteswapper(MODHeaderXC5 &self, bool) {
  FByteswapper(static_cast<MODHeaderCommon &>(self));
  FByteswapper(self.vertexBufferSize);
  FByteswapper(self.numTextures);
  FByteswapper(self.numGroups);
  FByteswapper(self.bones);
  FByteswapper(self.groups);
  FByteswapper(self.textures);
  FByteswapper(self.meshes);
  FByteswapper(self.vertexBuffer);
  FByteswapper(self.indices);
}

template <> void FByteswapper(MODHeaderXDx32 &self, bool) {
  FByteswapper(static_cast<MODHeaderCommon &>(self));
  FByteswapper(self.vertexBufferSize);
  FByteswapper(self.numTextures);
  FByteswapper(self.numGroups);
  FByteswapper(self.bones);
  FByteswapper(self.groups);
  FByteswapper(self.materialHashes);
  FByteswapper(self.meshes);
  FByteswapper(self.vertexBuffer);
  FByteswapper(self.indices);
  FByteswapper(self.dataEnd);
}

template <> void FByteswapper(MODHeaderX70 &self, bool) {
  FByteswapper(static_cast<MODHeaderCommon &>(self));
  FByteswapper(self.vertexBufferSize);
  FByteswapper(self.unkBufferSize);
  FByteswapper(self.numTextures);
  FByteswapper(self.bones);
  FByteswapper(self.textures);
  FByteswapper(self.meshes);
  FByteswapper(self.vertexBuffer);
  FByteswapper(self.unkBuffer);
  FByteswapper(self.indices);
}

template <> void FByteswapper(MODHeaderXE5 &self, bool) {
  FByteswapper(static_cast<MODHeaderCommon &>(self));
  FByteswapper(self.vertexBufferSize);
  FByteswapper(self.numTextures);
  FByteswapper(self.numGroups);
  FByteswapper(self.numSkins);
  FByteswapper(self.bones);
  FByteswapper(self.groups);
  FByteswapper(self.materials);
  FByteswapper(self.meshes);
  FByteswapper(self.vertexBuffer);
  FByteswapper(self.indices);
}

template <> void FByteswapper(MODHeaderX170 &self, bool) {
  FByteswapper(static_cast<MODHeaderCommon &>(self));
  FByteswapper(self.vertexBufferSize);
  FByteswapper(self.unkBufferSize);
  FByteswapper(self.numTextures);
  FByteswapper(self.bones);
  FByteswapper(self.textures);
  FByteswapper(self.meshes);
  FByteswapper(self.vertexBuffer);
  FByteswapper(self.unkBuffer);
  FByteswapper(self.indices);
}

template <> void FByteswapper(MODHeaderX21 &self, bool) {
  FByteswapper(static_cast<MODHeaderCommon &>(self));
  FByteswapper(self.vertexBufferSize);
  FByteswapper(self.numGroups);
  FByteswapper(self.numSkins);
  FByteswapper(self.bones);
  FByteswapper(self.groups);
  FByteswapper(self.materials);
  FByteswapper(self.meshes);
  FByteswapper(self.vertexBuffer);
  FByteswapper(self.indices);
  FByteswapper(self.unkSize);
  FByteswapper(self.unkData);
}

template <> void FByteswapper(MODMaterialX70 &self, bool way) {
  FByteswapper(self.pshData);
  FByteswapper(self.vshData, way);
  FByteswapper(self.baseTextureIndex);
  FByteswapper(self.normalTextureIndex);
  FByteswapper(self.maskTextureIndex);
  FByteswapper(self.lightTextureIndex);
  FByteswapper(self.shadowTextureIndex);
  FByteswapper(self.additionalTextureIndex);
  FByteswapper(self.cubeMapTextureIndex);
  FByteswapper(self.detailTextureIndex);
  FByteswapper(self.AOTextureIndex);
  FByteswapper(self.transparency);
  FByteswapper(self.fresnelFactor);
  FByteswapper(self.fresnelBias);
  FByteswapper(self.specularPower);
  FByteswapper(self.envMapPower);
  FByteswapper(self.lightMapScale);
  FByteswapper(self.detailFactor);
  FByteswapper(self.detailWrap);
  FByteswapper(self.envMapBias);
  FByteswapper(self.normalBias);
  FByteswapper(self.transmit);
  FByteswapper(self.paralax);
  FByteswapper(self.hash);
}

template <> void FByteswapper(MODMaterialX170 &self, bool) {
  FByteswapper(self.pshData);
  FByteswapper(self.vshData);
  FByteswapper(self.baseTextureIndex);
  FByteswapper(self.normalTextureIndex);
  FByteswapper(self.maskTextureIndex);
  FByteswapper(self.lightTextureIndex);
  FByteswapper(self.shadowTextureIndex);
  FByteswapper(self.additionalTextureIndex);
  FByteswapper(self.cubeMapTextureIndex);
  FByteswapper(self.detailTextureIndex);
  FByteswapper(self.AOTextureIndex);
  FByteswapper(self.transparency);
  FByteswapper(self.unk00);
  FByteswapper(self.fresnelFactor);
  FByteswapper(self.fresnelBias);
  FByteswapper(self.specularPower);
  FByteswapper(self.envMapPower);
  FByteswapper(self.lightMapScale);
  FByteswapper(self.detailFactor);
  FByteswapper(self.detailWrap);
  FByteswapper(self.envMapBias);
  FByteswapper(self.normalBias);
  FByteswapper(self.transmit);
  FByteswapper(self.paralax);
  FByteswapper(self.hash);
}

template <> void FByteswapper(MODMaterialXC5 &self, bool way) {
  FByteswapper(self.pshData, way);
  FByteswapper(self.vshData, way);
  FByteswapper(self.baseTextureIndex);
  FByteswapper(self.normalTextureIndex);
  FByteswapper(self.maskTextureIndex);
  FByteswapper(self.lightTextureIndex);
  FByteswapper(self.shadowTextureIndex);
  FByteswapper(self.additionalTextureIndex);
  FByteswapper(self.cubeMapTextureIndex);
  FByteswapper(self.detailTextureIndex);
  FByteswapper(self.AOTextureIndex);
  FByteswapper(self.transparency);
  FByteswapper(self.unk01);
  FByteswapper(self.specularPower);
  FByteswapper(self.envMapPower);
  FByteswapper(self.lightMapScale);
  FByteswapper(self.detailFactor);
  FByteswapper(self.detailWrap);
  FByteswapper(self.envMapBias);
  FByteswapper(self.normalBias);
  FByteswapper(self.unk02);
  FByteswapper(self.unk03);
  FByteswapper(self.unk04);
  FByteswapper(self.unk05);
  FByteswapper(self.unk06);
  FByteswapper(self.unk07);
}

template <> void FByteswapper(MODMaterialHash &self, bool) {
  FByteswapper(self.hash);
}

template <> void FByteswapper(MODMeshX99 &self, bool) {
  FByteswapper(self.unk);
  FByteswapper(self.materialIndex);
  FByteswapper(self.numVertices);
  FByteswapper(self.endIndex);
  FByteswapper(self.vertexStart);
  FByteswapper(self.vertexStreamOffset);
  FByteswapper(self.vertexStream2Offset);
  FByteswapper(self.indexStart);
  FByteswapper(self.numIndices);
  FByteswapper(self.indexValueOffset);
  FByteswapper(self.startIndex);
  FByteswapper(self.skinInfo);
}

template <> void FByteswapper(MODMeshX70 &self, bool) {
  FByteswapper(self.unk);
  FByteswapper(self.materialIndex);
  FByteswapper(self.numVertices);
  FByteswapper(self.vertexStart);
  FByteswapper(self.vertexStreamOffset);
  FByteswapper(self.vertexStream2Offset);
  FByteswapper(self.indexStart);
  FByteswapper(self.numIndices);
  FByteswapper(self.indexValueOffset);
  FByteswapper(self.bboxMin);
  FByteswapper(self.bboxMax);
}

template <> void FByteswapper(MODMeshXC5 &self, bool way) {
  FByteswapper(self.unk);
  FByteswapper(self.numVertices);
  FByteswapper(self.data0, way);
  FByteswapper(self.data1, way);
  FByteswapper(self.vertexStart);
  FByteswapper(self.vertexStreamOffset);
  FByteswapper(self.vertexFormat);
  FByteswapper(self.indexStart);
  FByteswapper(self.numIndices);
  FByteswapper(self.indexValueOffset);
  FByteswapper(self.numEnvelopes);
  FByteswapper(self.meshIndex);
  FByteswapper(self.minVertex);
  FByteswapper(self.maxVertex);
  FByteswapper(self.hash);
}

template <> void FByteswapper(MODMeshXD2 &self, bool way) {
  FByteswapper(self.unk);
  FByteswapper(self.numVertices);
  FByteswapper(self.data0, way);
  FByteswapper(self.data1, way);
  FByteswapper(self.vertexStart);
  FByteswapper(self.vertexStreamOffset);
  FByteswapper(self.vertexFormat);
  FByteswapper(self.indexStart);
  FByteswapper(self.numIndices);
  FByteswapper(self.indexValueOffset);
  FByteswapper(self.meshIndex);
  FByteswapper(self.minVertex);
  FByteswapper(self.maxVertex);
  FByteswapper(self.unk);
}

template <> void FByteswapper(MODMeshXE5 &self, bool way) {
  FByteswapper<MODMeshXD2>(self, way);
}

#pragma endregion
#pragma region Savers
void SaveMODX99(const MODInner<MODTraitsX99LE> &main, BinWritterRef wr) {
  MODHeaderX99 header{};
  header.id = MODID;
  header.version = 0x99;
  wr.Push();
  wr.Skip(sizeof(header));
  wr.ApplyPadding();
  wr.Skip(sizeof(main.bounds) + sizeof(main.metadata));
  wr.ApplyPadding();

  header.numBones = main.bones.size();

  if (header.numBones) {
    header.bones = wr.Tell();
    header.numBoneMaps = main.skinRemaps.size();
    wr.WriteContainer(main.bones);
    wr.WriteContainer(main.refPoses);
    wr.WriteContainer(main.transforms);
    wr.Write(main.remaps);
    wr.WriteContainer(main.skinRemaps);
  }

  wr.ApplyPadding();
  header.numGroups = main.groups.size();

  if (header.numGroups) {
    header.groups = wr.Tell();
    wr.WriteContainer(main.groups);
  }

  wr.ApplyPadding();
  header.numTextures = main.paths.size();
  header.numMaterials = main.materials.size();

  if (header.numTextures || header.numMaterials) {
    header.textures = wr.Tell();
    for (auto &p : main.paths) {
      wr.WriteContainer(p);
      wr.Skip(MODTraitsX99LE::pathSize - p.size());
    }
    wr.WriteContainer(main.materials);
  }

  wr.ApplyPadding();
  header.numMeshes = main.meshes.size();
  header.meshes = wr.Tell();
  wr.WriteContainer(main.meshes);
  wr.WriteContainerWCount(main.envelopes);
  wr.ApplyPadding();
  header.vertexBufferSize = main.vertexBuffer.size() - main.unkBufferSize;
  header.unkBufferSize = main.unkBufferSize;
  header.vertexBuffer = wr.Tell();
  wr.WriteBuffer(main.vertexBuffer.data(), header.vertexBufferSize);

  if (header.unkBufferSize) {
    wr.ApplyPadding();
    header.unkBuffer = wr.Tell();
    wr.WriteBuffer(main.vertexBuffer.data() + header.vertexBufferSize,
                   header.unkBufferSize);
  }

  wr.ApplyPadding();
  header.indices = wr.Tell();
  header.numIndices = main.indexBuffer.size() + 1;
  wr.WriteContainer(main.indexBuffer);
  const size_t eof = wr.Tell();
  wr.Pop();
  wr.Write(header);
  wr.ApplyPadding();
  wr.Write(main.bounds);
  wr.Write(main.metadata);
  wr.Seek(eof);
  wr.ApplyPadding();
}

void SaveMODXC5(const MODInner<MODTraitsXC5> &main, BinWritterRef wr) {
  MODHeaderXC5 header{};
  header.id = MODID;
  header.version = 0xC5;
  wr.Push();
  wr.Skip(sizeof(header));
  wr.ApplyPadding();
  wr.Skip(sizeof(main.bounds) + sizeof(main.metadata));
  wr.ApplyPadding();

  header.numBones = main.bones.size();

  if (header.numBones) {
    header.bones = wr.Tell();
    wr.WriteContainer(main.bones);
    wr.WriteContainer(main.refPoses);
    wr.WriteContainer(main.transforms);
    wr.Write(main.remaps);
  }

  wr.ApplyPadding();
  header.numGroups = main.groups.size();

  if (header.numGroups) {
    header.groups = wr.Tell();
    wr.WriteContainer(main.groups);
  }

  wr.ApplyPadding();
  header.numTextures = main.paths.size();
  header.numMaterials = main.materials.size();

  if (header.numTextures || header.numMaterials) {
    header.textures = wr.Tell();
    for (auto &p : main.paths) {
      wr.WriteContainer(p);
      wr.Skip(MODTraitsXC5::pathSize - p.size());
    }
    wr.WriteContainer(main.materials);
  }

  wr.ApplyPadding();
  header.numMeshes = main.meshes.size();
  header.meshes = wr.Tell();
  wr.WriteContainer(main.meshes);
  wr.WriteContainerWCount(main.envelopes);
  wr.ApplyPadding();
  header.vertexBufferSize = main.vertexBuffer.size();
  header.vertexBuffer = wr.Tell();
  wr.WriteContainer(main.vertexBuffer);
  wr.ApplyPadding();
  header.indices = wr.Tell();
  header.numIndices = main.indexBuffer.size();
  wr.WriteContainer(main.indexBuffer);
  const size_t eof = wr.Tell();
  wr.Pop();
  wr.Write(header);
  wr.ApplyPadding();
  wr.Write(main.bounds);
  wr.Write(main.metadata);
  wr.Seek(eof);
  wr.ApplyPadding();
}
#pragma endregion
#pragma region Loaders

template <class Header, class Traits>
MODImpl::ptr LoadMODX70(BinReaderRef_e rd) {
  Header header;
  MODInner<Traits> main;
  rd.Read(header);
  rd.ApplyPadding();
  rd.Read(main.bounds);
  rd.Read(main.metadata);
  header.numIndices--;

  if (header.numBones) {
    rd.Seek(header.bones);
    rd.ReadContainer(main.bones, header.numBones);
    rd.ReadContainer(main.refPoses, header.numBones);
    rd.ReadContainer(main.transforms, header.numBones);
  }

  rd.Seek(header.textures);
  rd.ReadContainerLambda(main.paths, header.numTextures,
                         [](BinReaderRef_e rd, std::string &p) {
                           MODPath<Traits::pathSize> path;
                           rd.Read(path);
                           p = path.path;
                         });
  rd.ReadContainer(main.materials, header.numMaterials);

  rd.Seek(header.meshes);
  rd.ReadContainer(main.meshes, header.numMeshes);

  main.unkBufferSize = header.unkBufferSize;

  main.vertexBuffer.resize(header.vertexBufferSize + main.unkBufferSize);

  rd.Seek(header.vertexBuffer);
  rd.ReadBuffer(main.vertexBuffer.data(), header.vertexBufferSize);

  if (header.unkBufferSize) {
    rd.Seek(header.unkBuffer);
    rd.ReadBuffer(main.vertexBuffer.data() + header.vertexBufferSize,
                  header.unkBufferSize);
  }

  rd.Seek(header.indices);
  rd.ReadContainer(main.indexBuffer, header.numIndices);

  return std::make_unique<decltype(main)>(std::move(main));
}

MODImpl::ptr LoadMODXC5(BinReaderRef_e rd) {
  MODHeaderXC5 header;
  MODInner<MODTraitsXC5> main;
  rd.Read(header);
  rd.ApplyPadding();
  rd.Read(main.bounds);
  rd.Read(main.metadata);

  if (header.numBones) {
    rd.Seek(header.bones);
    rd.ReadContainer(main.bones, header.numBones);
    rd.ReadContainer(main.refPoses, header.numBones);
    rd.ReadContainer(main.transforms, header.numBones);
    rd.Read(main.remaps);
  }

  if (header.numGroups) {
    rd.Seek(header.groups);
    rd.ReadContainer(main.groups, header.numGroups);
  }

  rd.Seek(header.textures);
  rd.ReadContainerLambda(main.paths, header.numTextures,
                         [](BinReaderRef_e rd, std::string &p) {
                           MODPath<MODTraitsXC5::pathSize> path;
                           rd.Read(path);
                           p = path.path;
                         });
  rd.ReadContainer(main.materials, header.numMaterials);

  rd.Seek(header.meshes);
  rd.ReadContainer(main.meshes, header.numMeshes);
  rd.ReadContainer(main.envelopes);

  rd.Seek(header.vertexBuffer);
  rd.ReadContainer(main.vertexBuffer, header.vertexBufferSize);

  rd.Seek(header.indices);
  rd.ReadContainer(main.indexBuffer, header.numIndices);

  return std::make_unique<decltype(main)>(std::move(main));
}

MODImpl::ptr LoadMODXC3(BinReaderRef_e rd) {
  MODHeaderXC5 header;
  MODInner<MODTraitsXC5> main;
  rd.Read(header);
  rd.ApplyPadding();
  rd.Read(main.bounds);
  rd.Read(main.metadata);

  if (header.numBones) {
    rd.Seek(header.bones);
    rd.ReadContainer(main.bones, header.numBones);
    rd.ReadContainer(main.refPoses, header.numBones);
    rd.ReadContainer(main.transforms, header.numBones);
    rd.Read(main.remaps);
  }

  if (header.numGroups) {
    rd.Seek(header.groups);
    rd.ReadContainer(main.groups, header.numGroups);
  }

  rd.Seek(header.textures);
  rd.ReadContainerLambda(main.paths, header.numTextures,
                         [](BinReaderRef_e rd, std::string &p) {
                           MODPath<MODTraitsXC5::pathSize> path;
                           rd.Read(path);
                           p = path.path;
                         });
  rd.ReadContainer(main.materials, header.numMaterials);

  rd.Seek(header.meshes);
  rd.ReadContainerLambda(main.meshes, header.numMeshes,
                         [](BinReaderRef_e rd, auto &m) {
                           rd.Read(m);
                           rd.Skip(8);
                         });
  rd.ReadContainer(main.envelopes);

  rd.Seek(header.vertexBuffer);
  rd.ReadContainer(main.vertexBuffer, header.vertexBufferSize);

  rd.Seek(header.indices);
  rd.ReadContainer(main.indexBuffer, header.numIndices);

  return std::make_unique<decltype(main)>(std::move(main));
}

template <class Traits> MODImpl::ptr LoadMODX99(BinReaderRef_e rd) {
  MODHeaderX99 header;
  MODInner<Traits> main;
  rd.Read(header);
  rd.ApplyPadding();
  rd.Read(main.bounds);
  rd.Read(main.metadata);
  header.numIndices--;

  if (header.numBones) {
    rd.Seek(header.bones);
    rd.ReadContainer(main.bones, header.numBones);
    rd.ReadContainer(main.refPoses, header.numBones);
    rd.ReadContainer(main.transforms, header.numBones);
    rd.Read(main.remaps);
    rd.ReadContainer(main.skinRemaps, header.numBoneMaps);
  }

  if (header.numGroups) {
    rd.Seek(header.groups);
    rd.ReadContainer(main.groups, header.numGroups);
  }

  rd.Seek(header.textures);
  rd.ReadContainerLambda(main.paths, header.numTextures,
                         [](BinReaderRef_e rd, std::string &p) {
                           MODPath<Traits::pathSize> path;
                           rd.Read(path);
                           p = path.path;
                         });
  rd.ReadContainer(main.materials, header.numMaterials);

  rd.Seek(header.meshes);
  rd.ReadContainer(main.meshes, header.numMeshes);
  rd.ReadContainer(main.envelopes);

  main.unkBufferSize = header.unkBufferSize;

  main.vertexBuffer.resize(header.vertexBufferSize + main.unkBufferSize);

  rd.Seek(header.vertexBuffer);
  rd.ReadBuffer(main.vertexBuffer.data(), header.vertexBufferSize);

  if (header.unkBufferSize) {
    rd.Seek(header.unkBuffer);
    rd.ReadBuffer(main.vertexBuffer.data() + header.vertexBufferSize,
                  header.unkBufferSize);
  }

  rd.Seek(header.indices);
  rd.ReadContainer(main.indexBuffer, header.numIndices);

  return std::make_unique<decltype(main)>(std::move(main));
}

template <class Traits> MODImpl::ptr LoadMODXD2x32(BinReaderRef_e rd) {
  MODHeaderXDx32 header;
  MODInner<Traits> main;
  rd.Read(header);
  rd.ApplyPadding();
  rd.Read(main.bounds);
  rd.Read(main.metadata);

  if (header.bones > 0 && header.bones <= 0x80) {
    // UMVC3 PS3 uses unique model
    throw std::runtime_error("Unsupported model format");
  }

  if (header.numBones) {
    rd.Seek(header.bones);
    rd.ReadContainer(main.bones, header.numBones);
    rd.ReadContainer(main.refPoses, header.numBones);
    rd.ReadContainer(main.transforms, header.numBones);
    rd.Read(main.remaps);
  }

  if (header.numGroups) {
    rd.Seek(header.groups);
    rd.ReadContainer(main.groups, header.numGroups);
  }

  rd.ReadContainer(main.materials, header.numMaterials);

  rd.Seek(header.meshes);
  rd.ReadContainerLambda(main.meshes, header.numMeshes,
                         [](BinReaderRef_e rd, auto &m) {
                           rd.Read(m);
                           rd.Skip(8);
                         });

  if constexpr (std::is_same_v<MODMetaDataV2, typename Traits::metadata>) {
    rd.ReadContainer(main.envelopes, main.metadata.numEnvelopes);
  } else {
    rd.ReadContainer(main.envelopes);
  }

  rd.Seek(header.vertexBuffer);
  rd.ReadContainer(main.vertexBuffer, header.vertexBufferSize);

  rd.Seek(header.indices);
  rd.ReadContainer(main.indexBuffer, header.numIndices);

  return std::make_unique<decltype(main)>(std::move(main));
}

template <class Traits> MODImpl::ptr LoadMODXDxLEx32(BinReaderRef_e rdn) {
  BinReaderRef rd(rdn);
  MODHeaderXDx32 header;
  MODInner<Traits> main;
  rd.Read(header);
  rd.ApplyPadding();
  rd.Read(main.bounds);
  rd.Read(main.metadata);

  if (header.numBones) {
    rd.Seek(header.bones);
    rd.ReadContainer(main.bones, header.numBones);
    rd.ReadContainer(main.refPoses, header.numBones);
    rd.ReadContainer(main.transforms, header.numBones);
    rd.Read(main.remaps);
  }

  if (header.numGroups) {
    rd.Seek(header.groups);
    rd.ReadContainer(main.groups, header.numGroups);
  }

  rdn.ReadContainer(main.materials, header.numMaterials);

  rd.Seek(header.meshes);
  rd.ReadContainer(main.meshes, header.numMeshes);

  if constexpr (std::is_same_v<MODMetaDataV2, typename Traits::metadata>) {
    rd.ReadContainer(main.envelopes, main.metadata.numEnvelopes);
  } else {
    rd.ReadContainer(main.envelopes);
  }

  rd.Seek(header.vertexBuffer);
  rd.ReadContainer(main.vertexBuffer, header.vertexBufferSize);

  rd.Seek(header.indices);
  rd.ReadContainer(main.indexBuffer, header.numIndices);

  return std::make_unique<decltype(main)>(std::move(main));
}

template <class Traits, class TraitsFallback>
MODImpl::ptr LoadMODXDxLE(BinReaderRef_e rdn) {
  BinReaderRef rd(rdn);
  MODHeaderXDx64 header;
  MODInner<Traits> main;
  rd.Push();
  rd.Read(header);
  rd.ApplyPadding();
  rd.Read(main.bounds);
  rd.Read(main.metadata);
  {
    uint64 maxPtr = header.bones | header.groups | header.materialNames |
                    header.meshes | header.vertexBuffer | header.indices |
                    header.dataEnd;
    size_t fileSize = rd.GetSize() << 1;

    if (maxPtr > fileSize) {
      rd.Pop();
      return LoadMODXDxLEx32<TraitsFallback>(rd);
    }
  }

  if (header.numBones) {
    rd.Seek(header.bones);
    rd.ReadContainer(main.bones, header.numBones);
    rd.ReadContainer(main.refPoses, header.numBones);
    rd.ReadContainer(main.transforms, header.numBones);
    rd.Read(main.remaps);
  }

  if (header.numGroups) {
    rd.Seek(header.groups);
    rd.ReadContainer(main.groups, header.numGroups);
  }

  rdn.ReadContainer(main.materials, header.numMaterials);

  rd.Seek(header.meshes);
  rdn.ReadContainerLambda(main.meshes, header.numMeshes,
                          [](BinReaderRef_e rd, auto &m) {
                            rd.Read(m);
                            rd.Skip(8);
                          });
  rd.ReadContainer(main.envelopes, main.metadata.numEnvelopes);

  rd.Seek(header.vertexBuffer);
  rd.ReadContainer(main.vertexBuffer, header.vertexBufferSize);

  rd.Seek(header.indices);
  rd.ReadContainer(main.indexBuffer, header.numIndices);

  return std::make_unique<decltype(main)>(std::move(main));
}

MODImpl::ptr LoadMODX06(BinReaderRef_e rdn) {
  MODHeaderX06 header;
  MODInner<MODTraitsX06> main;
  BinReaderRef rd(rdn);
  rd.Push();
  rd.Read(header);
  rd.ApplyPadding();
  rd.Read(main.bounds);
  rd.Read(main.metadata);

  if (header.numBones) {
    rd.Seek(header.bones);
    rd.ReadContainer(main.bones, header.numBones);
    rd.ReadContainer(main.refPoses, header.numBones);
    rd.ReadContainer(main.transforms, header.numBones);
    rd.Read(main.remaps);
    rd.ReadContainer(main.skinRemaps, header.numSkins);
  }

  if (header.numGroups) {
    rd.Seek(header.groups);
    rd.ReadContainer(main.groups, header.numGroups);
  }

  rdn.ReadContainer(main.materials, header.numMaterials);

  rd.Seek(header.meshes);
  rdn.ReadContainerLambda(main.meshes, header.numMeshes,
                          [](BinReaderRef_e rd, auto &m) {
                            rd.Read(m);
                            rd.Skip(8);
                          });
  rd.ReadContainer(main.envelopes);

  rd.Seek(header.vertexBuffer);
  rd.ReadContainer(main.vertexBuffer, header.vertexBufferSize);

  rd.Seek(header.indices);
  rd.ReadContainer(main.indexBuffer, header.numIndices);

  return std::make_unique<decltype(main)>(std::move(main));
}

MODImpl::ptr LoadMODXE5(BinReaderRef_e rd) {
  MODHeaderXE5 header;
  MODInner<MODTraitsXE5> main;
  rd.Push();
  rd.Read(header);
  rd.ApplyPadding();
  rd.Read(main.bounds);
  rd.Read(main.metadata);

  if (header.numBones) {
    rd.Seek(header.bones);
    rd.ReadContainer(main.bones, header.numBones);
    rd.ReadContainer(main.refPoses, header.numBones);
    rd.ReadContainer(main.transforms, header.numBones);
    rd.Read(main.remaps);
    rd.ReadContainer(main.skinRemaps, header.numSkins);
  }

  if (header.numGroups) {
    rd.Seek(header.groups);
    rd.ReadContainer(main.groups, header.numGroups);
  }

  rd.ReadContainer(main.materials, header.numMaterials);

  rd.Seek(header.meshes);
  rd.ReadContainer(main.meshes, header.numMeshes);
  rd.ReadContainer(main.envelopes);

  rd.Seek(header.vertexBuffer);
  rd.ReadContainer(main.vertexBuffer, header.vertexBufferSize);

  rd.Seek(header.indices);
  rd.ReadContainer(main.indexBuffer, header.numIndices);

  return std::make_unique<decltype(main)>(std::move(main));
}

MODImpl::ptr LoadMODXFF2C(BinReaderRef_e rd) {
  MODHeaderXE5 header;
  MODInner<MODTraitsXD3LE> main;
  rd.Push();
  rd.Read(header);
  rd.ApplyPadding();
  rd.Read(main.bounds);
  rd.Read(main.metadata);

  if (header.numBones) {
    rd.Seek(header.bones);
    rd.ReadContainer(main.bones, header.numBones);
    rd.ReadContainer(main.refPoses, header.numBones);
    rd.ReadContainer(main.transforms, header.numBones);
    rd.Read(main.remaps);
  }

  if (header.numGroups) {
    rd.Seek(header.groups);
    rd.ReadContainer(main.groups, header.numGroups);
  }

  rd.ReadContainer(main.materials, header.numMaterials);

  rd.Seek(header.meshes);
  rd.ReadContainerLambda(main.meshes, header.numMeshes,
                         [](BinReaderRef_e rd, auto &m) {
                           rd.Read(m);
                           rd.Skip(8);
                         });
  rd.ReadContainer(main.envelopes);

  rd.Seek(header.vertexBuffer);
  rd.ReadContainer(main.vertexBuffer, header.vertexBufferSize);

  rd.Seek(header.indices);
  rd.ReadContainer(main.indexBuffer, header.numIndices);

  return std::make_unique<decltype(main)>(std::move(main));
}

std::vector<MODMaterialProxy<MODMaterialX21>>
XFSToMaterials(const revil::XFS &main) {
  pugi::xml_document root;
  main.ToXML(root);
  std::vector<MODMaterialProxy<MODMaterialX21>> retval;
  auto mtArr = root.child("class").child("array");
  retval.resize(mtArr.attribute("count").as_int());

  for (uint32 curMat = 0; auto &c : mtArr.children()) {
    auto &mat = retval.at(curMat++).main;
    mat.name = "Material_";
    mat.name.append(c.find_child_by_attribute("name", "mTagID")
                        .attribute("value")
                        .as_string());
  }

  return retval;
}

MODImpl::ptr LoadMODX21(BinReaderRef_e rd) {
  MODHeaderX21 header;
  MODInner<MODTraitsX21> main;
  rd.Push();
  rd.Read(header);
  rd.ApplyPadding();
  rd.Read(main.bounds);
  rd.Read(main.metadata);

  if (header.numBones) {
    rd.Seek(header.bones);
    rd.ReadContainer(main.bones, header.numBones);
    rd.ReadContainer(main.refPoses, header.numBones);
    rd.ReadContainer(main.transforms, header.numBones);
    rd.Read(main.remaps);
    // skins??
  }

  if (header.numGroups) {
    rd.Seek(header.groups);
    rd.ReadContainer(main.groups, header.numGroups);
  }

  rd.Seek(header.materials);
  revil::XFS materials;
  materials.Load(rd, true);
  main.materials = XFSToMaterials(materials);

  rd.Seek(header.meshes);
  rd.ReadContainer(main.meshes, header.numMeshes);
  rd.ReadContainer(main.envelopes);

  rd.Seek(header.vertexBuffer);
  rd.ReadContainer(main.vertexBuffer, header.vertexBufferSize);

  rd.Seek(header.indices);
  rd.ReadContainer(main.indexBuffer, header.numIndices);

  return std::make_unique<decltype(main)>(std::move(main));
}

#pragma endregion

bool MODMaker::operator<(const MODMaker &i0) const {
  return reinterpret_cast<const uint64 &>(*this) <
         reinterpret_cast<const uint64 &>(i0);
}

static const std::map<MODMaker, MODImpl::ptr (*)(BinReaderRef_e)> modLoaders{
    {{MODVersion::X70, true}, LoadMODX70<MODHeaderX70, MODTraitsX70>},
    {{MODVersion::X170}, LoadMODX70<MODHeaderX170, MODTraitsX170>},
    {{MODVersion::X99}, LoadMODX99<MODTraitsX99LE>},
    {{MODVersion::X19C}, LoadMODX99<MODTraitsX99LE>},
    {{MODVersion::X99, true}, LoadMODX99<MODTraitsX99BE>},
    {{MODVersion::XC3, true}, LoadMODXC3},
    {{MODVersion::XC5}, LoadMODXC5},
    {{MODVersion::XD2}, LoadMODXDxLEx32<MODTraitsXD2>},
    {{MODVersion::XD3}, LoadMODXDxLE<MODTraitsXD3x64, MODTraitsXD3LE>},
    {{MODVersion::XD6},
     LoadMODXDxLE<MODTraitsXD6, MODTraitsXD3LE>}, // unused fallback
    {{MODVersion::XD4},
     LoadMODXDxLEx32<MODTraitsXD3PS4>}, // todo normals (different traits)

    {{MODVersion::XD2, true}, LoadMODXD2x32<MODTraitsXD2>},
    {{MODVersion::XD3, true}, LoadMODXD2x32<MODTraitsXD2>},
    {{MODVersion::XD4, true}, LoadMODXD2x32<MODTraitsXD2>},
    {{MODVersion::X05}, LoadMODX06},
    {{MODVersion::X06}, LoadMODX06},
    {{MODVersion::XE5, true}, LoadMODXE5},
    {{MODVersion::XE5}, LoadMODXE5},
    {{MODVersion::XE6, true}, LoadMODXE5},
    {{MODVersion::XE6}, LoadMODXE5},
    {{MODVersion::XE7}, LoadMODXE5},
    {{MODVersion::XFF2C, true}, LoadMODXFF2C},
    {{MODVersion::X21, true}, LoadMODX21},
};

template <class C> MODImpl::ptr makeMod() {
  return std::make_unique<MODInner<C>>();
}

static const std::map<MODMaker, MODImpl::ptr (*)()> modMakers{
    {{MODVersion::X70, true}, makeMod<MODTraitsX70>},
};

MOD::MOD(MODMaker make) {
  auto found = modMakers.find(make);

  if (es::IsEnd(modMakers, found)) {
    throw std::runtime_error("Cannon find specified MODMaker instance.");
  }

  pi = found->second();
}

void MOD::Load(const std::string &fileName) {
  BinReader rd(fileName);
  Load(rd);
}

void MOD::Load(BinReaderRef_e rd) {
  MODHeaderCommon header;
  rd.Push();
  rd.Read(header);
  rd.Pop();

  if (header.id == DOMID || header.id == RMDID) {
    rd.SwapEndian(true);
    FByteswapper(header);
  } else if (header.id != MODID) {
    throw es::InvalidHeaderError(header.id);
  }

  MODMaker mk;
  mk.version = static_cast<MODVersion>(header.version);
  mk.swappedEndian = rd.SwappedEndian();

  auto found = modLoaders.find(mk);

  if (es::IsEnd(modLoaders, found)) {
    throw es::InvalidVersionError(mk.version);
  }

  pi = found->second(rd);
  pi->Reflect(rd.SwappedEndian());
}
