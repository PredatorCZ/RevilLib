/*  DWM2GLTF
    Copyright(C) 2021-2022 Lukas Cone

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

#include "project.h"
#include "re_common.hpp"
#include "spike/gltf.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/io/binwritter_stream.hpp"
#include "spike/io/fileinfo.hpp"
#include "spike/type/matrix44.hpp"
#include "spike/uni/model.hpp"
#include "spike/util/aabb.hpp"

std::string_view filters[]{".dwm$"};

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = DWM2GLTF_DESC " v" DWM2GLTF_VERSION ", " DWM2GLTF_COPYRIGHT
                            "Lukas Cone",
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

struct DWModel {
  uint32 unk[14];
  void NoSwap();
};

struct VTPosition {
  Vector pos;
  float weight;

  void NoSwap();
};

struct Morph {
  float id;
  float value;

  void NoSwap();
};

void AppProcessFile(AppContext *ctx) {
  BinReaderRef rd(ctx->GetStream());
  GLTFModel main;
  uint32 id;
  uint32 version;
  uint32 numVertices;
  rd.Read(id);
  rd.Read(version);
  rd.Read(numVertices);
  rd.Skip(numVertices * 4);
  DWModel dwModel;
  rd.Read(dwModel);

  auto &posStream = main.GetVt12();
  auto [acc, accId] = main.NewAccessor(posStream, 4);
  acc.count = numVertices;
  acc.type = gltf::Accessor::Type::Vec3;
  acc.componentType = gltf::Accessor::ComponentType::Float;

  gltf::Primitive prim;
  prim.attributes["POSITION"] = accId;
  prim.mode = gltf::Primitive::Mode::Points;
  std::vector<VTPosition> positions;
  rd.ReadContainer(positions, numVertices);

  for (size_t i = 0; i < numVertices; i++) {
    Morph mph;
    rd.Read(mph);
    auto pos = positions.at(i).pos;
    pos.x = mph.value;
    posStream.wr.Write(pos);
  }

  gltf::Mesh mesh;
  mesh.primitives.emplace_back(std::move(prim));
  gltf::Node node;
  node.mesh = main.meshes.size();
  main.meshes.emplace_back(std::move(mesh));

  main.scenes.front().nodes.emplace_back(main.nodes.size());
  main.nodes.emplace_back(node);

  BinWritterRef wr(ctx->NewFile(ctx->workingFile.ChangeExtension(".glb")).str);

  main.FinishAndSave(wr, std::string(ctx->workingFile.GetFolder()));
}
