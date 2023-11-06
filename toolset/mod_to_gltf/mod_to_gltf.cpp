/*  MOD2GLTF
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
#include "revil/mod.hpp"
#include "spike/gltf.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/io/binwritter_stream.hpp"
#include "spike/io/fileinfo.hpp"
#include "spike/type/matrix44.hpp"
#include "spike/uni/model.hpp"
#include "spike/uni/skeleton.hpp"
#include "spike/util/aabb.hpp"

std::string_view filters[]{".mod$", ".dom$"};

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = MOD2GLTF_DESC " v" MOD2GLTF_VERSION ", " MOD2GLTF_COPYRIGHT
                            "Lukas Cone",
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

struct MODGLTF : GLTFModel {
public:
  using GLTF::FinishAndSave;

  void ProcessSkeletons(const uni::Skeleton &skel);
  void ProcessModel(const uni::Model &model);
  void Pipeline(const revil::MOD &model);
  size_t MakeSkin(const uni::Skin &skin);

  GLTFStream &AnimStream() {
    if (aniStream < 0) {
      auto &newStream = NewStream("anims");
      aniStream = newStream.slot;
      return newStream;
    }
    return Stream(aniStream);
  }

private:
  gltf::Attributes SaveVertices(const uni::VertexArray &vtArray);
  std::vector<int32> skeleton;

  int32 aniStream = -1;
};

static const float SCALE = 0.01;

void MODGLTF::ProcessSkeletons(const uni::Skeleton &skel) {
  gltf::Node rootNode;
  rootNode.name = "reference";

  for (auto b : skel) {
    gltf::Node bone;
    es::Matrix44 value;
    b->GetTM(value);
    Vector4A16 translation, rotation, scale;
    value.Decompose(translation, rotation, scale);
    translation *= SCALE;
    memcpy(bone.translation.data(), &translation, sizeof(bone.translation));
    memcpy(bone.rotation.data(), &rotation, sizeof(bone.rotation));
    memcpy(bone.scale.data(), &scale, sizeof(bone.scale));
    auto parent = b->Parent();
    auto index = nodes.size();
    revil::BoneIndex hash = b->Index();
    bone.name = std::to_string(index) + ':' + std::to_string(hash.motIndex);

    if (parent) {
      revil::BoneIndex pid = parent->Index();
      nodes[pid.id].children.push_back(index);
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

size_t MODGLTF::MakeSkin(const uni::Skin &skin) {
  auto &str = SkinStream();
  size_t retval = skins.size();
  skins.emplace_back();
  auto &gskin = skins.back();
  auto [acc, index] = NewAccessor(str, 16);
  auto &wr = str.wr;
  gskin.inverseBindMatrices = index;
  acc.componentType = gltf::Accessor::ComponentType::Float;
  acc.count = skin.NumNodes();
  acc.type = gltf::Accessor::Type::Mat4;

  for (size_t i = 0; i < acc.count; i++) {
    gskin.joints.emplace_back(skeleton.at(skin.NodeIndex(i)));
    es::Matrix44 bindOffset;
    skin.GetTM(bindOffset, i);
    bindOffset.r1() *= SCALE;
    bindOffset.r2() *= SCALE;
    bindOffset.r3() *= SCALE;
    bindOffset.r4() *= SCALE;
    bindOffset.r4().W = 1;
    wr.Write(bindOffset);
  }

  return retval;
}

gltf::Attributes MODGLTF::SaveVertices(const uni::VertexArray &vtArray) {
  size_t vertexCount = vtArray.NumVertices();
  auto descs = vtArray.Descriptors();
  gltf::Attributes attrs;
  std::vector<UCVector4> weights[2];
  std::vector<UCVector4> bones[2];
  size_t curWeight = 0;

  for (auto d : *descs) {
    using u = uni::PrimitiveDescriptor::Usage_e;
    switch (d->Usage()) {
    case u::Position: {
      WritePositions(attrs, *d, vertexCount);
      break;
    }

    case u::Normal: {
      attrs["NORMAL"] = WriteNormals8(*d, vertexCount);
      break;
    }

    case u::TextureCoordiante: {
      WriteTexCoord(attrs, *d, vertexCount);
      break;
    }

    case u::VertexColor: {
      WriteVertexColor(attrs, *d, vertexCount);
      break;
    }

    case u::Tangent: {
      auto &stream = GetVt4();
      auto [acc, index] = NewAccessor(stream, 1);
      acc.count = vertexCount;
      acc.componentType = gltf::Accessor::ComponentType::Byte;
      acc.normalized = true;
      acc.type = gltf::Accessor::Type::Vec4;
      attrs["TANGENT"] = index;

      uni::FormatCodec::fvec sampled;
      d->Codec().Sample(sampled, d->RawBuffer(), vertexCount, d->Stride());

      for (auto &v : sampled) {
        auto pure = v * Vector4A16(1.f, 1.f, 1.f, 0.f);
        pure.Normalize() *= 0x7f;
        pure = _mm_round_ps(pure._data, _MM_ROUND_NEAREST);
        CVector4 comp = pure.Convert<int8>();
        if (!v.w) {
          comp.w = 0x7f;
        } else {
          comp.w = -0x7f;
        }

        stream.wr.Write(comp);
      }

      break;
    }

    case u::BoneIndices: {
      if (d->Type().outType == uni::FormatType::FLOAT) {
        uni::FormatCodec::fvec sampled;
        d->Codec().Sample(sampled, d->RawBuffer(), vertexCount, d->Stride());
        d->Resample(sampled);

        auto &curBones = bones[d->Index()];
        if (curBones.empty()) {
          curBones.resize(vertexCount);
        }

        for (size_t index = 0; auto &v : sampled) {
          curBones[index++] = v.Convert<uint8>();
        }
      } else {
        uni::FormatCodec::ivec sampled;
        d->Codec().Sample(sampled, d->RawBuffer(), vertexCount, d->Stride());

        if (d->UnpackDataType() ==
            uni::PrimitiveDescriptor::UnpackDataType_e::Add) {
          IVector4A16 udata(d->UnpackData().min);

          for (auto &v : sampled) {
            v += udata;
          }
        }

        auto &curBones = bones[d->Index()];
        if (curBones.empty()) {
          curBones.resize(vertexCount);
        }

        for (size_t index = 0; auto &v : sampled) {
          curBones[index++] = v.Convert<uint8>();
        }
      }

      break;
    }

    case u::BoneWeights: {
      static constexpr size_t fmtNumElements[]{
          0, 4, 3, 4, 2, 3, 1, 2, 4, 3, 4, 2, 3, 2, 1, 3, 4, 1,
      };
      size_t numElems = fmtNumElements[uint8(d->Type().compType)];
      uni::FormatCodec::fvec sampled;
      d->Codec().Sample(sampled, d->RawBuffer(), vertexCount, d->Stride());

      for (size_t e = 0; e < numElems; e++, curWeight++) {
        auto &curWts = weights[curWeight / 4];
        if (curWts.empty()) {
          curWts.resize(vertexCount);
        }

        for (size_t index = 0; auto &v : sampled) {
          auto weight = std::round(v[e] * 0xff);
          curWts[index++][curWeight % 4] = weight;
        }
      }

      break;
    }

    default:
      break;
    }
  }

  // Eliminate error where bone has zero weight
  auto WriteBones = [&](size_t bnIndex) {
    auto &stream = GetVt4();
    auto [acc, index] = NewAccessor(stream, 1);
    acc.count = vertexCount;
    acc.componentType = gltf::Accessor::ComponentType::UnsignedByte;
    acc.type = gltf::Accessor::Type::Vec4;

    auto name = "JOINTS_" + std::to_string(bnIndex);
    attrs[name] = index;

    auto &curWeights = weights[bnIndex];
    auto &curBones = bones[bnIndex];

    for (size_t v = 0; v < vertexCount; v++) {
      UCVector4 cw = curWeights[v];
      UCVector4 cb = curBones[v];
      cb *= UCVector4(cw[0] > 0, cw[1] > 0, cw[2] > 0, cw[3] > 0);
      stream.wr.Write(cb);
    }
  };

  auto WriteWeights = [&](size_t wtIndex) {
    auto &stream = GetVt4();
    auto [acc, index] = NewAccessor(stream, 1);
    acc.count = vertexCount;
    acc.componentType = gltf::Accessor::ComponentType::UnsignedByte;
    acc.normalized = true;
    acc.type = gltf::Accessor::Type::Vec4;

    auto name = "WEIGHTS_" + std::to_string(wtIndex);
    attrs[name] = index;

    stream.wr.WriteContainer(weights[wtIndex]);
  };

  const size_t numWeights = weights[0].size();

  if (!weights[1].empty()) {
    if (curWeight < 8) {
      for (size_t i = 0; i < numWeights; i++) {
        auto &wt0 = weights[0].at(i);
        auto &wt1 = weights[1].at(i);

        wt1[3] = std::max(0xff - wt0[3] - wt0[2] - wt0[1] - wt0[0] - wt1[2] -
                              wt1[1] - wt1[0],
                          0);
      }
    }
    WriteWeights(0);
    WriteBones(0);
    WriteWeights(1);
    WriteBones(1);
  } else if (!weights[0].empty()) {
    if (curWeight == 3) {
      for (size_t i = 0; i < numWeights; i++) {
        auto &wt0 = weights[0].at(i);
        wt0[3] = std::max(0xff - wt0[2] - wt0[1] - wt0[0], 0);
      }
    } else if (curWeight == 2) {
      for (size_t i = 0; i < numWeights; i++) {
        auto &wt0 = weights[0].at(i);
        wt0[2] = std::max(0xff - wt0[1] - wt0[0], 0);
      }
    } else if (curWeight == 1) {
      for (size_t i = 0; i < numWeights; i++) {
        auto &wt0 = weights[0].at(i);
        wt0[1] = std::max(0xff - wt0[0], 0);
      }
    }

    WriteWeights(0);
    WriteBones(0);
  } else if (!bones[0].empty()) {
    weights[0].insert(weights[0].begin(), bones[0].size(), {0xff, 0, 0, 0});
    WriteWeights(0);
    WriteBones(0);
  }

  return attrs;
}

std::string LODName(revil::LODIndex &lod) {
  std::string retval("LOD");

  if (lod.lod1) {
    retval.push_back('1');
  }
  if (lod.lod2) {
    retval.push_back('2');
  }
  if (lod.lod3) {
    retval.push_back('3');
  }

  return retval;
}

void MODGLTF::ProcessModel(const uni::Model &model) {
  for (auto materials = model.Materials(); auto m : *materials) {
    gltf::Material gMat;
    gMat.name = m->Name();
    this->materials.emplace_back(std::move(gMat));
  }

  std::map<std::string, size_t> lodNodes;

  auto LODNode = [&](revil::LODIndex &lod, size_t gnode) {
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

  auto primitives = model.Primitives();
  auto skins = model.Skins();
  std::vector<size_t> indicesIndices;
  std::vector<gltf::Attributes> verticesIndices;
  std::vector<size_t> skinIndices;

  for (auto s : *skins) {
    skinIndices.emplace_back(MakeSkin(*s));
  }

  for (auto indices = model.Indices(); auto i : *indices) {
    indicesIndices.emplace_back(SaveIndices(*i));
  }

  for (auto vertices = model.Vertices(); auto i : *vertices) {
    verticesIndices.emplace_back(SaveVertices(*i));
  }

  for (auto p : *primitives) {
    auto &attrs = verticesIndices.at(p->VertexArrayIndex(0));
    if (attrs.empty()) {
      continue;
    }
    const size_t gnodeIndex = nodes.size();
    nodes.emplace_back();
    auto &gnode = nodes.back();
    gnode.mesh = meshes.size();
    gnode.name = "Mesh[" + p->Name() + "]";
    revil::LODIndex lod(p->LODIndex());

    if (!skinIndices.empty()) {
      gnode.skin = skinIndices.at(p->SkinIndex());
      gnode.name = LODName(lod) + "|" + gnode.name;
      scenes.back().nodes.push_back(gnodeIndex);
    } else {
      gnode.scale.fill(SCALE);
      LODNode(lod, gnodeIndex);
    }

    meshes.emplace_back();
    auto &gmesh = meshes.back();
    gmesh.primitives.emplace_back();
    gmesh.name = gnode.name;
    auto &prim = gmesh.primitives.back();
    prim.indices = indicesIndices.at(p->IndexArrayIndex());
    prim.attributes = attrs;
    prim.material = p->MaterialIndex();
    prim.mode = [&] {
      switch (p->IndexType()) {
      case uni::Primitive::IndexType_e::Triangle:
        return gltf::Primitive::Mode::Triangles;
      case uni::Primitive::IndexType_e::Strip:
        return gltf::Primitive::Mode::TriangleStrip;
      default:
        throw std::runtime_error("Invalid index type");
      }
    }();
  }

  /*lod0Node.extensionsAndExtras["extensions"]["MSFT_lod"]["ids"] = {
      lodNodesBegin + 1, lodNodesBegin + 2};
  lod0Node.extensionsAndExtras["extras"]["MSFT_screencoverage"] = {
      0.5,
      0.2,
      0.01,
  };*/
}

void MODGLTF::Pipeline(const revil::MOD &model) {
  extensionsRequired.emplace_back("KHR_mesh_quantization");
  extensionsUsed.emplace_back("KHR_mesh_quantization");
  // doc.extensionsUsed.emplace_back("MSFT_lod");
  auto skel = model.As<uni::Element<const uni::Skeleton>>();
  ProcessSkeletons(*skel.get());
  auto mod = model.As<uni::Element<const uni::Model>>();
  ProcessModel(*mod.get());
}

void AppProcessFile(AppContext *ctx) {
  revil::MOD mod;
  mod.Load(ctx->GetStream());
  MODGLTF main;
  main.Pipeline(mod);
  BinWritterRef wr(ctx->NewFile(ctx->workingFile.ChangeExtension(".glb")).str);

  main.FinishAndSave(wr, std::string(ctx->workingFile.GetFolder()));
}
