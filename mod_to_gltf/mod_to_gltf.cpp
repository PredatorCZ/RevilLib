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

#include "datas/binreader_stream.hpp"
#include "datas/binwritter.hpp"
#include "datas/fileinfo.hpp"
#include "datas/matrix44.hpp"
#include "gltf.hpp"
#include "project.h"
#include "re_common.hpp"
#include "revil/mod.hpp"
#include "uni/model.hpp"
#include "uni/skeleton.hpp"

es::string_view filters[]{
    ".mod$",
    {},
};

static AppInfo_s appInfo{
    AppInfo_s::CONTEXT_VERSION,
    AppMode_e::CONVERT,
    ArchiveLoadType::FILTERED,
    MOD2GLTF_DESC " v" MOD2GLTF_VERSION ", " MOD2GLTF_COPYRIGHT "Lukas Cone",
    nullptr,
    filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

class MODGLTF : GLTF {
public:
  std::vector<es::Matrix44> globalTMs;

  using GLTF::FinishAndSave;

  void ProcessSkeletons(const uni::Skeleton &skel);
  void ProcessModel(const uni::Model &model);
  void Pipeline(const revil::MOD &model);
  size_t MakeSkin(const std::map<uint8, uint8> &usedBones,
                  const uni::Skin &skin, const es::Matrix44 &transform);

private:
  GLTFStream &SkinStream() {
    if (skinStreamSlot < 0) {
      auto &newStream = NewStream("skin-ibms");
      skinStreamSlot = newStream.slot;
      return newStream;
    }
    return Stream(skinStreamSlot);
  }

  GLTFStream &VertexStream(size_t lod) {
    if (lodVertices[lod] < 0) {
      auto &newStream = NewStream("vertices-lod" + std::to_string(lod));
      lodVertices[lod] = newStream.slot;
      return newStream;
    }
    return Stream(lodVertices[lod]);
  }

  GLTFStream &VertexPosStream(size_t lod) {
    if (lodVertexPos[lod] < 0) {
      auto &newStream = NewStream("vertpos-lod" + std::to_string(lod), 8);
      lodVertexPos[lod] = newStream.slot;
      return newStream;
    }
    return Stream(lodVertexPos[lod]);
  }

  GLTFStream &IndexStream(size_t lod) {
    if (lodIndices[lod] < 0) {
      auto &newStream = NewStream("indices-lod" + std::to_string(lod));
      lodIndices[lod] = newStream.slot;
      return newStream;
    }
    return Stream(lodIndices[lod]);
  }

  size_t SaveIndices(const uni::Primitive &prim, size_t lod);

  int32 skinStreamSlot = -1;
  int32 lodVertices[3]{-1, -1, -1};
  int32 lodVertexPos[3]{-1, -1, -1};
  int32 lodIndices[3]{-1, -1, -1};
};

void MODGLTF::ProcessSkeletons(const uni::Skeleton &skel) {
  const size_t startIndex = nodes.size();

  for (auto b : skel) {
    gltf::Node bone;
    es::Matrix44 value;
    b->GetTM(value);
    globalTMs.push_back(value);
    memcpy(bone.matrix.data(), &value, sizeof(value));
    auto parent = b->Parent();
    auto index = nodes.size();
    revil::BoneIndex hash = b->Index();
    bone.name = std::to_string(index) + ':' + std::to_string(hash.motIndex);

    if (parent) {
      revil::BoneIndex pid = parent->Index();
      nodes[pid.id].children.push_back(index);
      globalTMs.back() = globalTMs.back() * globalTMs.at(pid.id);
    }

    nodes.emplace_back(std::move(bone));
  }

  if (nodes.size() != startIndex) {
    scenes.back().nodes.push_back(startIndex);
  }
}

size_t MODGLTF::SaveIndices(const uni::Primitive &prim, size_t lod) {
  auto &stream = IndexStream(lod);
  auto [acc, index] = NewAccessor(stream, 2);
  size_t indexCount = prim.NumIndices();
  auto indicesRaw = prim.RawIndexBuffer();
  auto indices = reinterpret_cast<const uint16 *>(indicesRaw);
  bool as8bit = true;

  for (size_t i = 0; i < indexCount; i++) {
    uint16 cur = indices[i];
    if (cur < 0xffff && cur >= 0xff) {
      as8bit = false;
      break;
    }
  }

  acc.componentType = as8bit ? gltf::Accessor::ComponentType::UnsignedByte
                              : gltf::Accessor::ComponentType::UnsignedShort;
  acc.type = gltf::Accessor::Type::Scalar;

  auto process = [&, &acc = acc](auto &data, uint32 reset) {
    data.reserve(indexCount);
    bool inverted = false;
    data.push_back(indices[0]);
    data.push_back(indices[1]);

    for (size_t i = 2; i < indexCount - 1; i++) {
      uint16 item = indices[i];

      if (item == reset) {
        data.push_back(indices[i - 1]);
        if (inverted) {
          data.push_back(indices[i + 1]);
          inverted = false;
        }
        data.push_back(indices[i + 1]);
      } else {
        data.push_back(item);
        inverted = !inverted;
      }
    }

    if (indices[indexCount - 1] != reset) {
      data.push_back(indices[indexCount - 1]);
    }

    using vtype =
        typename std::remove_reference<decltype(data)>::type::value_type;
    acc.count = data.size();
    auto buffer = reinterpret_cast<const char *>(data.data());
    stream.wr.WriteBuffer(buffer, data.size() * sizeof(vtype));
  };

  if (as8bit) {
    std::vector<uint8> data;
    process(data, 0xffff);
  } else {
    std::vector<uint16> data;
    process(data, 0xffff);
  }

  return index;
}

size_t MODGLTF::MakeSkin(const std::map<uint8, uint8> &usedBones,
                         const uni::Skin &skin, const es::Matrix44 &transform) {
  auto &str = SkinStream();
  size_t retval = skins.size();
  skins.emplace_back();
  auto &gskin = skins.back();
  auto [acc, index] = NewAccessor(str, 16);
  auto &wr = str.wr;
  gskin.inverseBindMatrices = index;
  acc.componentType = gltf::Accessor::ComponentType::Float;
  acc.count = usedBones.size();
  acc.type = gltf::Accessor::Type::Mat4;

  std::map<uint8, uint8> invertedBones;

  for (auto &b : usedBones) {
    invertedBones[b.second] = b.first;
  }

  for (auto b : invertedBones) {
    size_t nodeIndex = skin.NodeIndex(b.second);
    es::Matrix44 gmtx = globalTMs.at(nodeIndex);
    es::Matrix44 bindOffset;
    skin.GetTM(bindOffset, b.second);
    auto originalOffset = gmtx * bindOffset;
    auto appliedTM = transform * originalOffset;
    auto ibm = -gmtx * appliedTM;
    wr.Write(ibm);
    gskin.joints.push_back(nodeIndex);
  }

  return retval;
}

struct AABBResult {
  Vector4A16 max;
  Vector4A16 min;
  Vector4A16 center;
};

AABBResult GetAABB(const std::vector<Vector4A16> &points) {
  Vector4A16 max(-INFINITY), min(INFINITY), center;
  for (auto &p : points) {
    max = Vector4A16(_mm_max_ps(max._data, p._data));
    min = Vector4A16(_mm_min_ps(min._data, p._data));
  }
  center = (max + min) * 0.5f;

  return {max, min, center};
}

void MODGLTF::ProcessModel(const uni::Model &model) {
  auto primitives = model.Primitives();
  auto skins = model.Skins();
  gltf::Node lod0Node{};
  lod0Node.name = "LOD-Near";
  gltf::Node lod1Node{};
  lod1Node.name = "LOD-Middle";
  gltf::Node lod2Node{};
  lod2Node.name = "LOD-Far";

  for (auto p : *primitives) {
    const size_t gnodeIndex = nodes.size();
    nodes.emplace_back();
    auto &gnode = nodes.back();
    gnode.mesh = meshes.size();
    meshes.emplace_back();
    auto &gmesh = meshes.back();
    gmesh.primitives.emplace_back();
    gmesh.name = p->Name();
    auto &prim = gmesh.primitives.back();
    revil::LODIndex lod(p->LODIndex());
    size_t lodIndex = lod.lod1 ? 0 : (lod.lod2 ? 1 : 2);
    prim.indices = SaveIndices(*p.get(), lodIndex);
    prim.mode = gltf::Primitive::Mode::TriangleStrip;
    std::map<uint8, uint8> boneIds;
    size_t vertexCount = p->NumVertices();
    auto descs = p->Descriptors();
    es::Matrix44 meshTransform;
    Vector4A16 meshScale;

    for (auto d : *descs) {
      using u = uni::PrimitiveDescriptor::Usage_e;
      switch (d->Usage()) {
      case u::Position: {
        auto &posStream = VertexPosStream(lodIndex);
        auto [acc, index] = NewAccessor(posStream, 4);
        acc.count = vertexCount;
        acc.componentType = gltf::Accessor::ComponentType::UnsignedShort;
        acc.normalized = true;
        acc.type = gltf::Accessor::Type::Vec3;
        acc.max = {0xffff, 0xffff, 0xffff};
        acc.min = {0, 0, 0};
        prim.attributes["POSITION"] = index;

        uni::FormatCodec::fvec sampled;
        d->Codec().Sample(sampled, d->RawBuffer(), vertexCount, d->Stride());
        d->Resample(sampled);

        auto aabb = GetAABB(sampled);
        auto &max = aabb.max;
        auto &offset = aabb.min;
        meshScale = max - offset;
        // FIX: Uniform scale to fix normal artifacts
        meshScale = Vector4A16(
            std::max(std::max(meshScale.x, meshScale.y), meshScale.z));
        meshTransform.r4() = offset;
        Vector4A16 invscale = ((Vector4A16(1.f) / meshScale) * 0xffff);
        for (auto &v : sampled) {
          Vector4A16 vl((v - offset) * invscale);
          USVector4 comp = vl.Convert<uint16>();
          posStream.wr.Write(comp);
        }

        break;
      }

      case u::Normal: {
        auto &stream = VertexStream(lodIndex);
        auto [acc, index] = NewAccessor(stream, 1);
        acc.count = vertexCount;
        acc.componentType = gltf::Accessor::ComponentType::Byte;
        acc.normalized = true;
        acc.type = gltf::Accessor::Type::Vec3;
        prim.attributes["NORMAL"] = index;

        uni::FormatCodec::fvec sampled;
        d->Codec().Sample(sampled, d->RawBuffer(), vertexCount, d->Stride());
        d->Resample(sampled);

        // auto normalized = Vector4A16(1.f) - meshScale.Normalized();
        auto corrector = Vector4A16(1.f, 1.f, 1.f, 0.f); // * normalized;
        // corrector.Normalize();

        for (auto &v : sampled) {
          auto pure = v * corrector;
          pure.Normalize() *= 0x7f;
          pure = _mm_round_ps(pure._data, _MM_ROUND_NEAREST);
          auto comp = pure.Convert<int8>();
          stream.wr.Write(comp);
        }

        break;
      }

      case u::Tangent: {
        auto &stream = VertexStream(lodIndex);
        auto [acc, index] = NewAccessor(stream, 1);
        acc.count = vertexCount;
        acc.componentType = gltf::Accessor::ComponentType::Byte;
        acc.normalized = true;
        acc.type = gltf::Accessor::Type::Vec4;
        prim.attributes["TANGENT"] = index;

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

      case u::TextureCoordiante: {
        auto &stream = VertexStream(lodIndex);
        auto [acc, index] = NewAccessor(stream, 4);
        acc.count = vertexCount;
        acc.type = gltf::Accessor::Type::Vec2;
        auto coordName = "TEXCOORD_" + std::to_string(d->Index());
        prim.attributes[coordName] = index;

        uni::FormatCodec::fvec sampled;
        d->Codec().Sample(sampled, d->RawBuffer(), vertexCount, d->Stride());
        d->Resample(sampled);

        auto aabb = GetAABB(sampled);
        auto &max = aabb.max;
        auto &min = aabb.min;
        auto vertWr = stream.wr;

        if (max <= 1.f && min >= -1.f) {
          if (min >= 0.f) {
            acc.componentType = gltf::Accessor::ComponentType::UnsignedShort;
            acc.normalized = true;

            for (auto &v : sampled) {
              USVector4 comp = Vector4A16(v * 0xffff).Convert<uint16>();
              vertWr.Write(USVector2(comp));
            }
          } else {
            acc.componentType = gltf::Accessor::ComponentType::Short;
            acc.normalized = true;

            for (auto &v : sampled) {
              SVector4 comp = Vector4A16(v * 0x7fff).Convert<int16>();
              vertWr.Write(SVector2(comp));
            }
          }
        } else {
          acc.componentType = gltf::Accessor::ComponentType::Float;

          for (auto &v : sampled) {
            vertWr.Write(Vector2(v));
          }
        }

        break;
      }

      case u::VertexColor: {
        auto &stream = VertexStream(lodIndex);
        auto [acc, index] = NewAccessor(stream, 1);
        acc.count = vertexCount;
        acc.componentType = gltf::Accessor::ComponentType::UnsignedByte;
        acc.normalized = true;
        acc.type = gltf::Accessor::Type::Vec4;
        auto coordName = "COLOR_" + std::to_string(d->Index());
        prim.attributes[coordName] = index;
        uni::FormatCodec::fvec sampled;
        d->Codec().Sample(sampled, d->RawBuffer(), vertexCount, d->Stride());

        for (auto &v : sampled) {
          stream.wr.Write((v * 0xff).Convert<uint8>());
        }

        break;
      }

      case u::BoneIndices: {
        auto &stream = VertexStream(lodIndex);
        auto [acc, index] = NewAccessor(stream, 1);
        acc.count = vertexCount;
        acc.componentType = gltf::Accessor::ComponentType::UnsignedByte;
        acc.type = gltf::Accessor::Type::Vec4;
        auto name = "JOINTS_" + std::to_string(d->Index());
        prim.attributes[name] = index;

        uni::FormatCodec::ivec sampled;
        d->Codec().Sample(sampled, d->RawBuffer(), vertexCount, d->Stride());

        for (auto &v : sampled) {
          auto outVal = v.Convert<uint8>();
          for (size_t e = 0; e < 4; e++) {
            if (!boneIds.count(outVal._arr[e])) {
              const size_t curIndex = boneIds.size();
              boneIds[outVal._arr[e]] = curIndex;
            }
            stream.wr.Write(boneIds[outVal._arr[e]]);
          }
        }

        break;
      }

      case u::BoneWeights: {
        auto &stream = VertexStream(lodIndex);
        auto [acc, index] = NewAccessor(stream, 1);
        acc.count = vertexCount;
        acc.componentType = gltf::Accessor::ComponentType::UnsignedByte;
        acc.normalized = true;
        acc.type = gltf::Accessor::Type::Vec4;

        auto name = "WEIGHTS_" + std::to_string(d->Index());
        prim.attributes[name] = index;

        uni::FormatCodec::fvec sampled;
        d->Codec().Sample(sampled, d->RawBuffer(), vertexCount, d->Stride());

        for (auto &v : sampled) {
          stream.wr.Write((v * 0xff).Convert<uint8>());
        }

        break;
      }

      default:
        break;
      }
    }

    if (skins->Size()) {
      auto skin = skins->At(p->SkinIndex());
      meshTransform.r1() *= meshScale.x;
      meshTransform.r2() *= meshScale.y;
      meshTransform.r3() *= meshScale.z;
      meshTransform.r4().w = 1.f;
      gnode.skin = MakeSkin(boneIds, *skin, meshTransform);
    } else {
      memcpy(gnode.scale.data(), &meshScale, sizeof(gnode.scale));
      memcpy(gnode.translation.data(), &meshTransform.r4(),
             sizeof(gnode.translation));
    }

    if (lod.lod1 && lod.lod2 && lod.lod3) {
      scenes.back().nodes.push_back(gnodeIndex);
    } else {
      if (lod.lod1) {
        lod0Node.children.push_back(gnodeIndex);
      }

      if (lod.lod2) {
        if (lod.lod1) {
          lod1Node.children.push_back(nodes.size());
          nodes.emplace_back(nodes[gnodeIndex]);
        } else {
          lod1Node.children.push_back(gnodeIndex);
        }
      }

      if (lod.lod3) {
        if (lod.lod1 || lod.lod2) {
          lod2Node.children.push_back(nodes.size());
          nodes.emplace_back(nodes[gnodeIndex]);
        } else {
          lod2Node.children.push_back(gnodeIndex);
        }
      }
    }
  }

  size_t lodNodesBegin = nodes.size();
  /*lod0Node.extensionsAndExtras["extensions"]["MSFT_lod"]["ids"] = {
      lodNodesBegin + 1, lodNodesBegin + 2};
  lod0Node.extensionsAndExtras["extras"]["MSFT_screencoverage"] = {
      0.5,
      0.2,
      0.01,
  };*/

  if (!lod0Node.children.empty()) {
    nodes.emplace_back(std::move(lod0Node));
    scenes.back().nodes.push_back(lodNodesBegin++);
  }

  if (!lod1Node.children.empty()) {
    nodes.emplace_back(std::move(lod1Node));
    scenes.back().nodes.push_back(lodNodesBegin++);
  }

  if (!lod2Node.children.empty()) {
    nodes.emplace_back(std::move(lod2Node));
    scenes.back().nodes.push_back(lodNodesBegin++);
  }
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

void AppProcessFile(std::istream &stream, AppContext *ctx) {
  revil::MOD mod;
  BinReaderRef rd(stream);
  mod.Load(rd);
  MODGLTF main;
  main.Pipeline(mod);
  AFileInfo outPath(ctx->outFile);
  BinWritter wr(outPath.GetFullPathNoExt().to_string() + ".glb");

  main.FinishAndSave(wr, outPath.GetFolder());
}
