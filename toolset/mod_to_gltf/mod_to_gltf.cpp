/*  MOD2GLTF
    Copyright(C) 2021-2026 Lukas Cone

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
#include "pugixml.hpp"
#include "re_common.hpp"
#include "revil/arc.hpp"
#include "revil/hashreg.hpp"
#include "revil/mod.hpp"
#include "spike/gltf.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/io/binwritter_stream.hpp"
#include <spanstream>

std::string_view filters[]{
    ".mod$",
    ".dom$",
    ".arc$",
};

struct MOD2GLTF : ReflectorBase<MOD2GLTF> {
  bool quantizeMesh = true;
  bool quantizeMeshFake = false;
  bool noLods = true;
  bool mergeMeshes = true;
  bool generateModelInfo = false;
} settings;

REFLECT(
    CLASS(MOD2GLTF),
    MEMBERNAME(quantizeMesh, "quantize-mesh", "q",
               ReflDesc{"Apply KHR_mesh_quantization."}),
    MEMBERNAME(
        quantizeMeshFake, "quantize-mesh-fake", "Q",
        ReflDesc{"KHR_mesh_quantization is not marked as required extension."}),
    MEMBERNAME(noLods, "no-lods", "l", ReflDesc{"Do not export LOD meshes."}),
    MEMBERNAME(mergeMeshes, "merge-meshes", "m",
               ReflDesc{"Merge meshes as groups"}),
    MEMBERNAME(generateModelInfo, "generate-model-info", "i",
               ReflDesc{"Generate model info xml alongside gltf."}), );

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = MOD2GLTF_DESC " v" MOD2GLTF_VERSION ", " MOD2GLTF_COPYRIGHT
                            "Lukas Cone",
    .settings = reinterpret_cast<ReflectorFriend *>(&settings),
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

struct MODGLTF : GLTFModel {
public:
  void ProcessSkeletons(std::span<const MODBone> bones,
                        std::span<const es::Matrix44> tms);
  void ProcessModel(const revil::MOD &model);
  void Pipeline(const revil::MOD &model);
  size_t MakeSkin(const revil::MODSkinJoints skin,
                  std::span<const es::Matrix44> binds);

private:
  std::vector<int32> skeleton;
};

static const float SCALE = 0.01;

void MODGLTF::ProcessSkeletons(std::span<const MODBone> bones,
                               std::span<const es::Matrix44> tms) {
  gltf::Node rootNode;
  rootNode.name = "reference";

  for (size_t curBone = 0; auto b : bones) {
    gltf::Node bone;
    Vector4A16 translation, rotation, scale;
    tms[curBone].Decompose(translation, rotation, scale);
    translation *= SCALE;
    memcpy(bone.translation.data(), &translation, sizeof(bone.translation));
    memcpy(bone.rotation.data(), &rotation, sizeof(bone.rotation));
    memcpy(bone.scale.data(), &scale, sizeof(bone.scale));
    auto index = nodes.size();
    bone.name = std::to_string(curBone++) + ':' + std::to_string(b.index);

    if (b.parentIndex < 0xffff) {
      nodes[b.parentIndex].children.push_back(index);
    } else {
      rootNode.children.push_back(index);
    }

    skeleton.emplace_back(nodes.size());
    nodes.emplace_back(std::move(bone));
  }

  for (auto &s : skeleton) {
    auto &cNode = nodes.at(s);
    if (cNode.children.empty()) {
      continue;
    }
    cNode.children.emplace_back(nodes.size());

    gltf::Node sNode;
    sNode.name = cNode.name + "_s";
    s = nodes.size();
    nodes.emplace_back(std::move(sNode));
  }

  if (!skeleton.empty()) {
    scenes.back().nodes.push_back(nodes.size());
    nodes.emplace_back(std::move(rootNode));
  }
}

size_t MODGLTF::MakeSkin(const revil::MODSkinJoints joints,
                         std::span<const es::Matrix44> binds) {
  auto &str = SkinStream();
  size_t retval = skins.size();
  skins.emplace_back();
  auto &gskin = skins.back();
  auto [acc, index] = NewAccessor(str, 16);
  auto &wr = str.wr;
  gskin.inverseBindMatrices = index;
  acc.componentType = gltf::Accessor::ComponentType::Float;
  acc.count = joints.empty() ? binds.size() : joints.size();
  acc.type = gltf::Accessor::Type::Mat4;

  for (size_t i = 0; i < acc.count; i++) {
    const size_t boneIndex = joints.empty() ? i : joints[i];
    gskin.joints.emplace_back(skeleton.at(boneIndex));
    es::Matrix44 bindOffset = binds[boneIndex];
    bindOffset.r1() *= SCALE;
    bindOffset.r2() *= SCALE;
    bindOffset.r3() *= SCALE;
    bindOffset.r4() *= SCALE;
    bindOffset.r4().W = 1;
    wr.Write(bindOffset);
  }

  return retval;
}

std::string LODName(const revil::MODPrimitive &lod) {
  std::string retval("LOD");
  using F = revil::MODPrimitive::Flags;

  if (lod.flags == F::Lod1) {
    retval.push_back('1');
  }
  if (lod.flags == F::Lod2) {
    retval.push_back('2');
  }
  if (lod.flags == F::Lod3) {
    retval.push_back('3');
  }

  return retval;
}

void MODGLTF::ProcessModel(const revil::MOD &model) {
  std::map<std::string, size_t> lodNodes;

  auto LODNode = [&](const revil::MODPrimitive &lod, size_t gnode) {
    const std::string lodName = LODName(lod);

    if (lodNodes.contains(lodName)) {
      nodes.at(lodNodes.at(lodName)).children.push_back(gnode);
      return;
    }

    gltf::Node lodNode{};
    lodNode.name = lodName;
    lodNode.children.push_back(gnode);
    scenes.back().nodes.push_back(nodes.size());
    lodNodes.emplace(lodName, nodes.size());
    nodes.emplace_back(lodNode);
  };

  std::map<uint32, size_t> indicesIndices;
  std::map<uint32, gltf::Attributes> verticesIndices;
  std::vector<size_t> skinIndices;

  if (model.SkinJoints().empty() && model.InverseBinds().size() > 0) {
    skinIndices.emplace_back(MakeSkin({}, model.InverseBinds()));
  }

  for (auto &s : model.SkinJoints()) {
    skinIndices.emplace_back(MakeSkin(s, model.InverseBinds()));
  }

  union ShareKey {
    struct {
      uint16 groupId;
      uint8 skinIndex;
      bool lod1;
      bool lod2;
      bool lod3;
      uint16 reserved = 0;
    };
    uint64 key;
  };

  static_assert(sizeof(ShareKey) == sizeof(uint64));

  std::map<uint64, gltf::Mesh> uniqMeshes;
  std::map<uint32, uint32> usedMaterials;
  using F = revil::MODPrimitive::Flags;

  for (auto &p : model.Primitives()) {
    if (p.flags != F::Lod1 && settings.noLods &&
        (p.flags == F::Lod2 || p.flags == F::Lod3)) {
      continue;
    }

    gltf::Primitive prim;

    if (verticesIndices.count(p.vertexIndex) == 0) {
      auto &i = model.Vertices()[p.vertexIndex];
      gltf::Attributes attrs =
          SaveVertices(i.buffer, i.numVertices, i.attrs, i.stride);
      verticesIndices.emplace(p.vertexIndex, attrs);
      prim.attributes = attrs;
    } else {
      prim.attributes = verticesIndices.at(p.vertexIndex);
    }

    if (prim.attributes.empty()) {
      continue;
    }

    if (indicesIndices.count(p.indexIndex) == 0) {
      auto &i = model.Indices()[p.indexIndex];
      const size_t accIndex = SaveIndices(i.data(), i.size()).accessorIndex;
      indicesIndices.emplace(p.indexIndex, accIndex);
      prim.indices = accIndex;
    } else {
      prim.indices = indicesIndices.at(p.indexIndex);
    }

    if (usedMaterials.count(p.materialIndex) == 0) {
      usedMaterials.emplace(p.materialIndex, materials.size());
      prim.material = materials.size();
      auto &m = model.Materials()[p.materialIndex];

      gltf::Material &gMat = materials.emplace_back();
      gMat.name = m->GetName();
      if (gMat.name.empty()) {
        gMat.name = "Material_" + std::to_string(prim.material);
      }
    } else {
      prim.material = usedMaterials.at(p.materialIndex);
    }

    prim.mode = p.flags == F::TriStrips ? gltf::Primitive::Mode::TriangleStrip
                                        : gltf::Primitive::Mode::Triangles;

    if (settings.mergeMeshes) {
      ShareKey key{{
          .groupId = p.groupId,
          .skinIndex = uint8(p.skinIndex),
          .lod1 = p.flags == F::Lod1,
          .lod2 = p.flags == F::Lod2,
          .lod3 = p.flags == F::Lod3,
      }};

      uniqMeshes[key.key].primitives.emplace_back(prim);
      continue;
    }

    const size_t gnodeIndex = nodes.size();
    gltf::Node &gnode = nodes.emplace_back();
    gnode.mesh = meshes.size();
    gnode.name = "Mesh[" + std::to_string(p.meshId) + ":" +
                 std::to_string(p.groupId) + "]";
    //+ "](" + std::to_string(p.layout) + ")";

    if (!skinIndices.empty()) {
      gnode.skin = skinIndices.at(p.skinIndex);
      if (!settings.noLods) {
        gnode.name = LODName(p) + "|" + gnode.name;
      }
      scenes.back().nodes.push_back(gnodeIndex);
    } else {
      gnode.scale.fill(SCALE);
      if (!settings.noLods) {
        LODNode(p, gnodeIndex);
      } else {
        scenes.back().nodes.push_back(gnodeIndex);
      }
    }

    auto &gmesh = meshes.emplace_back();
    gmesh.primitives.emplace_back(prim);
    gmesh.name = gnode.name;
  }

  if (settings.mergeMeshes) {
    for (auto &[key, mesh] : uniqMeshes) {
      ShareKey skey{
          .key = key,
      };

      const size_t gnodeIndex = nodes.size();
      gltf::Node &gnode = nodes.emplace_back();
      gnode.mesh = meshes.size();
      gnode.name = "Group[" + std::to_string(skey.groupId) + "]";
      mesh.name = gnode.name;
      meshes.emplace_back(mesh);

      revil::MODPrimitive mp;
      mp.flags.Set(F::Lod1, skey.lod1);
      mp.flags.Set(F::Lod2, skey.lod2);
      mp.flags.Set(F::Lod3, skey.lod3);

      if (!skinIndices.empty()) {
        gnode.skin = skinIndices.at(skey.skinIndex);
        if (!settings.noLods) {
          gnode.name = LODName(mp) + "|" + gnode.name;
        }
        scenes.back().nodes.push_back(gnodeIndex);
      } else {
        gnode.scale.fill(SCALE);
        if (!settings.noLods) {
          LODNode(mp, gnodeIndex);
        } else {
          scenes.back().nodes.push_back(gnodeIndex);
        }
      }
    }
  }
}

void MODGLTF::Pipeline(const revil::MOD &model) {
  if (settings.quantizeMesh) {
    QuantizeMesh(settings.quantizeMeshFake);
  }

  ProcessSkeletons(model.Bones(), model.Transforms());
  ProcessModel(model);
}

void Convert(std::istream &str, const std::string &path, AppContext *ctx) {
  revil::MOD mod;
  mod.Load(str);
  MODGLTF main;
  main.Pipeline(mod);

  std::string fullPath(ctx->workingFile.GetFolder());
  fullPath.append(path);
  fullPath.append(".glb");

  BinWritterRef wr(ctx->NewFile(fullPath).str);

  main.FinishAndSave(wr, std::string(ctx->workingFile.GetFolder()));

  if (!settings.generateModelInfo) {
    return;
  }

  fullPath = ctx->workingFile.GetFolder();
  fullPath.append(path);
  fullPath.append(".mdi.xml");

  auto &outStr = ctx->NewFile(fullPath).str;

  pugi::xml_document doc;
  mod.ToXML(doc);
  doc.save(outStr);
}

struct ExtractContext : ArcExtractContext {
  AppContext *parentCtx;
  std::string curFile;

  void NewFile(const std::string &path) override {
    curFile = AFileInfo(path).GetFullPathNoExt();
  }
  void SendData(std::string_view data) override {
    std::ispanstream spstr(std::span<const char>(data.data(), data.size()));
    Convert(spstr, curFile, parentCtx);
  }
};

void AppProcessFile(AppContext *ctx) {
  if (ctx->workingFile.GetExtension() == ".arc") {
    static const std::set<uint32> filter{
        MTHashV1("rModel"),
        MTHashV2("rModel"),
    };

    ExtractContext ectx;
    ectx.parentCtx = ctx;

    EnumerateArchive(
        ctx->GetStream(), Platform::Auto, "lp", [&] { return &ectx; }, filter);
    return;
  }

  Convert(ctx->GetStream(), std::string(ctx->workingFile.GetFilenameExt()),
          ctx);
}
