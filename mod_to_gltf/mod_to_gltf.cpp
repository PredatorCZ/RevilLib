/*  MOD2GLTF
    Copyright(C) 2021 Lukas Cone

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
#include "gltf.h"
#include "project.h"
#include "re_common.hpp"
#include "revil/mod.hpp"
#include "uni/model.hpp"
#include "uni/skeleton.hpp"
#include <sstream>

using json = nlohmann::json;
using namespace fx;

static struct { bool binaryGLTF = false; } debug;

static struct MOD2GLTF : ReflectorBase<MOD2GLTF> {
} settings;

REFLECT(CLASS(MOD2GLTF));

es::string_view filters[]{
    "$.mod",
    {},
};

ES_EXPORT AppInfo_s appInfo{
    AppInfo_s::CONTEXT_VERSION,
    AppMode_e::CONVERT,
    ArchiveLoadType::FILTERED,
    MOD2GLTF_DESC " v" MOD2GLTF_VERSION ", " MOD2GLTF_COPYRIGHT "Lukas Cone",
    reinterpret_cast<ReflectorFriend *>(&settings),
    filters,
};

struct GLTFStream {
  std::stringstream str;
  BinWritterRef wr{str};
  size_t slot;
  size_t stride = 0;
  GLTFStream() = delete;
  GLTFStream(const GLTFStream &) = delete;
  GLTFStream(GLTFStream &&o) : str{std::move(o.str)}, wr{str}, slot(o.slot) {}
  GLTFStream &operator=(GLTFStream &&) = delete;
  GLTFStream &operator=(const GLTFStream &) = delete;
  GLTFStream(size_t slot_) : slot(slot_) {}
  GLTFStream(size_t slot_, size_t stride_) : slot(slot_), stride(stride_) {}
};

struct GLTFVertexStream {
  GLTFStream positions;
  GLTFStream normals;
  GLTFStream other;

  GLTFVertexStream() = delete;
  GLTFVertexStream(const GLTFVertexStream &) = delete;
  GLTFVertexStream(GLTFVertexStream &&) = default;
  GLTFVertexStream &operator=(GLTFVertexStream &&) = delete;
  GLTFVertexStream &operator=(const GLTFVertexStream &) = delete;
  GLTFVertexStream(size_t slot)
      : positions(slot, 8), normals(slot + 1, 4), other(slot + 2) {}
};

struct GLTFLODStream {
  GLTFVertexStream vertices;
  GLTFStream indices;
  GLTFLODStream() = delete;
  GLTFLODStream(const GLTFLODStream &) = delete;
  GLTFLODStream(GLTFLODStream &&) = default;
  GLTFLODStream &operator=(GLTFLODStream &&) = delete;
  GLTFLODStream &operator=(const GLTFLODStream &) = delete;
  GLTFLODStream(size_t slot) : vertices(slot), indices(slot + 3) {}
};

class GLTF {
public:
  GLTFStream main{0};
  GLTFLODStream lods[3]{{1}, {5}, {9}};
  gltf::Document doc;
  std::vector<esMatrix44> globalTMs;

  GLTF();
  void ProcessSkeletons(const uni::Skeleton &skel);
  void ProcessModel(const uni::Model &model);
  void Pipeline(const revil::MOD &model);
  size_t MakeSkin(const std::map<uint8, uint8> &usedBones,
                  const uni::Skin &skin, const esMatrix44 &transform);
};

GLTF::GLTF() { doc.scenes.emplace_back(); }

void GLTF::ProcessSkeletons(const uni::Skeleton &skel) {
  const size_t startIndex = doc.nodes.size();

  for (auto b : skel) {
    gltf::Node bone;
    esMatrix44 value;
    b->GetTM(value);
    globalTMs.push_back(value);
    memcpy(bone.matrix.data(), &value, sizeof(value));
    bone.name = b->Name();
    auto parent = b->Parent();
    auto index = doc.nodes.size();
    revil::BoneIndex hash = b->Index();
    bone.extensionsAndExtras["extras"]["motIndex"] = hash.motIndex;

    if (parent) {
      revil::BoneIndex pid = parent->Index();
      doc.nodes[pid.id].children.push_back(index);
      globalTMs.back() = globalTMs.at(pid.id) * globalTMs.back();
    }

    doc.nodes.emplace_back(std::move(bone));
  }

  if (doc.nodes.size() != startIndex) {
    doc.scenes.back().nodes.push_back(startIndex);
  }
}

size_t SaveIndices(const uni::Primitive &prim, gltf::Document &doc,
                   BinWritterRef wr, size_t bufferView) {
  wr.ApplyPadding();
  size_t retval = doc.accessors.size();
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

  doc.accessors.emplace_back();
  auto &cacc = doc.accessors.back();
  cacc.bufferView = bufferView;
  cacc.componentType = as8bit ? gltf::Accessor::ComponentType::UnsignedByte
                              : gltf::Accessor::ComponentType::UnsignedShort;
  cacc.type = gltf::Accessor::Type::Scalar;
  cacc.byteOffset = wr.Tell();

  auto process = [&](auto &data, uint32 reset) {
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
    cacc.count = data.size();
    auto buffer = reinterpret_cast<const char *>(data.data());
    wr.WriteBuffer(buffer, data.size() * sizeof(vtype));
  };

  if (as8bit) {
    std::vector<uint8> data;
    process(data, 0xffff);
  } else {
    std::vector<uint16> data;
    process(data, 0xffff);
  }

  return retval;
}

size_t GLTF::MakeSkin(const std::map<uint8, uint8> &usedBones,
                      const uni::Skin &skin, const esMatrix44 &transform) {
  auto wr = main.wr;
  wr.ApplyPadding();
  size_t retval = doc.skins.size();
  doc.skins.emplace_back();
  auto &gskin = doc.skins.back();
  gskin.inverseBindMatrices = doc.accessors.size();
  doc.accessors.emplace_back();
  auto &cacc = doc.accessors.back();
  cacc.bufferView = main.slot;
  cacc.componentType = gltf::Accessor::ComponentType::Float;
  cacc.count = usedBones.size();
  cacc.type = gltf::Accessor::Type::Mat4;
  cacc.byteOffset = wr.Tell();

  std::map<uint8, uint8> invertedBones;

  for (auto &b : usedBones) {
    invertedBones[b.second] = b.first;
  }

  for (auto b : invertedBones) {
    size_t nodeIndex = skin.NodeIndex(b.second);
    esMatrix44 gmtx = globalTMs.at(nodeIndex);
    esMatrix44 bindOffset;
    skin.GetTM(bindOffset, b.second);
    auto originalOffset = bindOffset * gmtx;
    auto appliedTM = originalOffset * transform;
    auto ibm = appliedTM * -gmtx;
    wr.Write(ibm);
    gskin.joints.push_back(nodeIndex);
  }

  return retval;
}

// r1() = max
// r2() = min
// r3() = center
esMatrix44 GetAABB(const std::vector<Vector4A16> &points) {
  Vector4A16 max(-INFINITY), min(INFINITY), center;
  for (auto &p : points) {
    max = Vector4A16(_mm_max_ps(max._data, p._data));
    min = Vector4A16(_mm_min_ps(min._data, p._data));
  }
  center = (max + min) * 0.5f;

  return {max, min, center, {}};
}

void GLTF::ProcessModel(const uni::Model &model) {
  auto primitives = model.Primitives();
  auto skins = model.Skins();
  gltf::Node lod0Node{};
  lod0Node.name = "LOD-Near";
  gltf::Node lod1Node{};
  lod1Node.name = "LOD-Middle";
  gltf::Node lod2Node{};
  lod2Node.name = "LOD-Far";

  for (auto p : *primitives) {
    const size_t gnodeIndex = doc.nodes.size();
    doc.nodes.emplace_back();
    auto &gnode = doc.nodes.back();
    gnode.mesh = doc.meshes.size();
    doc.meshes.emplace_back();
    auto &gmesh = doc.meshes.back();
    gmesh.primitives.emplace_back();
    gmesh.name = p->Name();
    auto &prim = gmesh.primitives.back();
    revil::LODIndex lod(p->LODIndex());
    size_t lodIndex = lod.lod3 ? 2 : (lod.lod2 ? 1 : 0);
    auto &lodStr = lods[lodIndex];
    prim.indices =
        SaveIndices(*p.get(), doc, lodStr.indices.wr, lodStr.indices.slot);
    prim.mode = gltf::Primitive::Mode::TriangleStrip;
    std::map<uint8, uint8> boneIds;
    size_t vertexCount = p->NumVertices();
    auto descs = p->Descriptors();
    esMatrix44 meshTransform;
    Vector4A16 meshScale;
    auto vertWr = lodStr.vertices.other.wr;

    for (auto d : *descs) {
      using u = uni::PrimitiveDescriptor::Usage_e;
      const size_t accessIndex = doc.accessors.size();
      doc.accessors.emplace_back();
      auto &cacc = doc.accessors.back();
      cacc.bufferView = lodStr.vertices.other.slot;
      vertWr.ApplyPadding(4);
      cacc.byteOffset = vertWr.Tell();
      cacc.count = vertexCount;

      switch (d->Usage()) {
      case u::Position: {
        uni::FormatCodec::fvec sampled;
        d->Codec().Sample(sampled, d->RawBuffer(), vertexCount, d->Stride());
        d->Resample(sampled);
        prim.attributes["POSITION"] = accessIndex;

        cacc.componentType = gltf::Accessor::ComponentType::UnsignedShort;
        cacc.normalized = true;
        cacc.type = gltf::Accessor::Type::Vec3;
        cacc.max = {0xffff, 0xffff, 0xffff};
        cacc.min = {0, 0, 0};
        cacc.bufferView = lodStr.vertices.positions.slot;

        auto aabb = GetAABB(sampled);
        auto &max = aabb.r1();
        auto &offset = aabb.r2();
        meshScale = max - offset;
        // FIX: Uniform scale to fix normal artifacts
        meshScale = Vector4A16(
            std::max(std::max(meshScale.x, meshScale.y), meshScale.z));
        meshTransform.r4() = offset;
        Vector4A16 invscale = ((Vector4A16(1.f) / meshScale) * 0xffff);
        auto posWr = lodStr.vertices.positions.wr;
        posWr.ApplyPadding(4);
        cacc.byteOffset = posWr.Tell();

        for (auto &v : sampled) {
          Vector4A16 vl((v - offset) * invscale);
          USVector4 comp = vl.Convert<uint16>();
          posWr.Write(comp);
        }

        break;
      }

      case u::Normal: {
        uni::FormatCodec::fvec sampled;
        d->Codec().Sample(sampled, d->RawBuffer(), vertexCount, d->Stride());
        d->Resample(sampled);
        prim.attributes["NORMAL"] = accessIndex;

        cacc.componentType = gltf::Accessor::ComponentType::Byte;
        cacc.normalized = true;
        cacc.type = gltf::Accessor::Type::Vec3;
        cacc.bufferView = lodStr.vertices.normals.slot;
        auto norWr = lodStr.vertices.normals.wr;
        norWr.ApplyPadding(4);
        cacc.byteOffset = norWr.Tell();

        // auto normalized = Vector4A16(1.f) - meshScale.Normalized();
        auto corrector = Vector4A16(1.f, 1.f, 1.f, 0.f); // * normalized;
        // corrector.Normalize();

        for (auto &v : sampled) {
          auto pure = v * corrector;
          pure.Normalize() *= 0x7f;
          pure = _mm_round_ps(pure._data, _MM_ROUND_NEAREST);
          auto comp = pure.Convert<int8>();
          norWr.Write(comp);
        }

        break;
      }

      case u::Tangent: {
        uni::FormatCodec::fvec sampled;
        d->Codec().Sample(sampled, d->RawBuffer(), vertexCount, d->Stride());
        prim.attributes["TANGENT"] = accessIndex;

        cacc.componentType = gltf::Accessor::ComponentType::Byte;
        cacc.normalized = true;
        cacc.type = gltf::Accessor::Type::Vec4;

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

          vertWr.Write(comp);
        }

        break;
      }

      case u::TextureCoordiante: {
        uni::FormatCodec::fvec sampled;
        d->Codec().Sample(sampled, d->RawBuffer(), vertexCount, d->Stride());
        d->Resample(sampled);
        auto coordName = "TEXCOORD_" + std::to_string(d->Index());
        prim.attributes[coordName] = accessIndex;

        auto aabb = GetAABB(sampled);
        auto &max = aabb.r1();
        auto &min = aabb.r2();

        if (max <= 1.f && min >= -1.f) {
          if (min >= 0.f) {
            cacc.componentType = gltf::Accessor::ComponentType::UnsignedShort;
            cacc.normalized = true;
            cacc.type = gltf::Accessor::Type::Vec2;

            for (auto &v : sampled) {
              USVector4 comp = Vector4A16(v * 0xffff).Convert<uint16>();
              vertWr.Write(USVector2(comp));
            }
          } else {
            cacc.componentType = gltf::Accessor::ComponentType::Short;
            cacc.normalized = true;
            cacc.type = gltf::Accessor::Type::Vec2;

            for (auto &v : sampled) {
              SVector4 comp = Vector4A16(v * 0x7fff).Convert<int16>();
              vertWr.Write(SVector2(comp));
            }
          }
        } else {
          cacc.componentType = gltf::Accessor::ComponentType::Float;
          cacc.type = gltf::Accessor::Type::Vec2;

          for (auto &v : sampled) {
            vertWr.Write(Vector2(v));
          }
        }

        break;
      }

      case u::VertexColor: {
        uni::FormatCodec::fvec sampled;
        d->Codec().Sample(sampled, d->RawBuffer(), vertexCount, d->Stride());
        auto coordName = "COLOR_" + std::to_string(d->Index());
        prim.attributes[coordName] = accessIndex;

        cacc.componentType = gltf::Accessor::ComponentType::UnsignedByte;
        cacc.normalized = true;
        cacc.type = gltf::Accessor::Type::Vec4;

        for (auto &v : sampled) {
          vertWr.Write((v * 0xff).Convert<uint8>());
        }

        break;
      }

      case u::BoneIndices: {
        uni::FormatCodec::ivec sampled;
        d->Codec().Sample(sampled, d->RawBuffer(), vertexCount, d->Stride());

        auto name = "JOINTS_" + std::to_string(d->Index());
        prim.attributes[name] = accessIndex;

        cacc.componentType = gltf::Accessor::ComponentType::UnsignedByte;
        cacc.type = gltf::Accessor::Type::Vec4;

        for (auto &v : sampled) {
          auto outVal = v.Convert<uint8>();
          for (size_t e = 0; e < 4; e++) {
            if (!boneIds.count(outVal._arr[e])) {
              const size_t curIndex = boneIds.size();
              boneIds[outVal._arr[e]] = curIndex;
            }
            vertWr.Write(boneIds[outVal._arr[e]]);
          }
        }

        break;
      }

      case u::BoneWeights: {
        uni::FormatCodec::fvec sampled;
        d->Codec().Sample(sampled, d->RawBuffer(), vertexCount, d->Stride());

        auto name = "WEIGHTS_" + std::to_string(d->Index());
        prim.attributes[name] = accessIndex;

        cacc.componentType = gltf::Accessor::ComponentType::UnsignedByte;
        cacc.normalized = true;
        cacc.type = gltf::Accessor::Type::Vec4;

        for (auto &v : sampled) {
          vertWr.Write((v * 0xff).Convert<uint8>());
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
      doc.scenes.back().nodes.push_back(gnodeIndex);
    } else {
      if (lod.lod1) {
        lod0Node.children.push_back(gnodeIndex);
      }

      if (lod.lod2) {
        if (lod.lod1) {
          lod1Node.children.push_back(doc.nodes.size());
          doc.nodes.emplace_back(doc.nodes[gnodeIndex]);
        } else {
          lod1Node.children.push_back(gnodeIndex);
        }
      }

      if (lod.lod3) {
        if (lod.lod1 || lod.lod2) {
          lod2Node.children.push_back(doc.nodes.size());
          doc.nodes.emplace_back(doc.nodes[gnodeIndex]);
        } else {
          lod2Node.children.push_back(gnodeIndex);
        }
      }
    }
  }

  size_t lodNodesBegin = doc.nodes.size();
  /*lod0Node.extensionsAndExtras["extensions"]["MSFT_lod"]["ids"] = {
      lodNodesBegin + 1, lodNodesBegin + 2};
  lod0Node.extensionsAndExtras["extras"]["MSFT_screencoverage"] = {
      0.5,
      0.2,
      0.01,
  };*/

  if (!lod0Node.children.empty()) {
    doc.nodes.emplace_back(std::move(lod0Node));
    doc.scenes.back().nodes.push_back(lodNodesBegin++);
  }

  if (!lod1Node.children.empty()) {
    doc.nodes.emplace_back(std::move(lod1Node));
    doc.scenes.back().nodes.push_back(lodNodesBegin++);
  }

  if (!lod2Node.children.empty()) {
    doc.nodes.emplace_back(std::move(lod2Node));
    doc.scenes.back().nodes.push_back(lodNodesBegin++);
  }
}

void GLTF::Pipeline(const revil::MOD &model) {
  doc.extensionsRequired.emplace_back("KHR_mesh_quantization");
  // doc.extensionsRequired.emplace_back("KHR_texture_transform");
  doc.extensionsUsed.emplace_back("KHR_mesh_quantization");
  // doc.extensionsUsed.emplace_back("MSFT_lod");
  // doc.extensionsUsed.emplace_back("KHR_texture_transform");
  auto skel = model.As<uni::Element<const uni::Skeleton>>();
  ProcessSkeletons(*skel.get());
  auto mod = model.As<uni::Element<const uni::Model>>();
  ProcessModel(*mod.get());

  size_t totalBufferSize = [&] {
    main.wr.ApplyPadding();
    size_t retval = main.wr.Tell();
    for (auto &l : lods) {
      l.vertices.positions.wr.ApplyPadding();
      l.vertices.normals.wr.ApplyPadding();
      l.vertices.other.wr.ApplyPadding();
      l.indices.wr.ApplyPadding();
      retval += l.vertices.positions.wr.Tell();
      retval += l.vertices.normals.wr.Tell();
      retval += l.vertices.other.wr.Tell();
      retval += l.indices.wr.Tell();
    }
    return retval;
  }();

  doc.buffers.emplace_back();
  auto &mainBuffer = doc.buffers.back();
  mainBuffer.byteLength = totalBufferSize;

  std::string buffer;
  buffer.reserve(totalBufferSize);

  auto appendBuffer = [&](auto name, auto &&storage, auto target) {
    doc.bufferViews.emplace_back();
    auto cBuffer = storage.str.str();
    auto &view = doc.bufferViews.back();
    view.buffer = 0;
    view.byteLength = cBuffer.size();
    view.byteOffset = buffer.size();
    view.byteStride = storage.stride;
    view.name = name;
    view.target = target;
    buffer.append(cBuffer);
  };

  appendBuffer("common-data", std::move(main),
               gltf::BufferView::TargetType::None);
  size_t lodIndex = 0;

  for (auto &l : lods) {
    auto lodName = "lod" + std::to_string(lodIndex);
    appendBuffer(lodName + "-positions", std::move(l.vertices.positions),
                 gltf::BufferView::TargetType::ArrayBuffer);
    appendBuffer(lodName + "-normals", std::move(l.vertices.normals),
                 gltf::BufferView::TargetType::ArrayBuffer);
    appendBuffer(lodName + "-other-vertices", std::move(l.vertices.other),
                 gltf::BufferView::TargetType::ArrayBuffer);
    appendBuffer(lodName + "-indices", std::move(l.indices),
                 gltf::BufferView::TargetType::ElementArrayBuffer);
    lodIndex++;
  }

  mainBuffer.data.resize(mainBuffer.byteLength);
  memcpy(mainBuffer.data.data(), buffer.data(), mainBuffer.byteLength);
}

void AppProcessFile(std::istream &stream, AppContext *ctx) {
  revil::MOD mod;
  BinReaderRef rd(stream);
  mod.Load(rd);
  GLTF main;
  main.Pipeline(mod);
  AFileInfo info(ctx->workingFile);
  auto outFile = info.GetFullPathNoExt().to_string() +
                 (debug.binaryGLTF ? ".glb" : ".gltf");

  gltf::Save(main.doc, outFile, debug.binaryGLTF);
}
