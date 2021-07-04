#include "datas/binreader.hpp"
#include "datas/binwritter.hpp"
#include "datas/directory_scanner.hpp"
#include "datas/fileinfo.hpp"
#include "datas/master_printer.hpp"
#include "datas/multi_thread.hpp"
#include "revil/re_asset.hpp"
#include <vector>

void LoadMOD(const std::string &fileName);
void LoadXFS(BinReaderRef rd);

#include "revil/tex.hpp"

#include "datas/reflector.hpp"

struct AtlasItem {
  std::string name;
  uint16 X;
  uint16 Y;
  uint16 numX;
  uint16 numY;
  uint32 bufferOffset;
};

REFLECTOR_CREATE(AtlasItem, 1, VARNAMES, name, X, Y, numX, numY);

struct InputItem {
  std::string name;
  std::string buffer;
  uint32 width;
  uint32 height;
  uint32 bpp;

  bool operator<(const InputItem &o) const {
    return width * height * bpp > o.width * o.height * o.bpp;
  }
};

struct AtlasCoord {
  uint16 x, y;

  bool operator<(AtlasCoord o) const {
    if (y == o.y) {
      return x < o.x;
    } else {
      return y < o.y;
    }
  }
};

#include "datas/pugiex.hpp"
#include "datas/reflector_xml.hpp"

struct IAtlas {
  std::string buffer;
  uint32 poolPixelsPerLine;
  uint32 lineStride;
  uint32 poolsPerLine;
  std::map<AtlasCoord, uint32> freePools;
  std::map<AtlasCoord, AtlasItem> items;

  IAtlas(uint32 blockSize, uint32 rectSize, uint32 bpp)
      : poolsPerLine(rectSize / blockSize), poolPixelsPerLine(blockSize),
        lineStride(rectSize * bpp) {
    buffer.resize(lineStride * rectSize);

    const size_t numPools = poolsPerLine * poolsPerLine;
    const size_t poolLineSize = poolPixelsPerLine * bpp;

    for (size_t p = 0; p < numPools; p++) {
      const uint16 posX = p % poolsPerLine;
      const uint16 posY = p / poolsPerLine;
      freePools[{posX, posY}] =
          (lineStride * posY * poolPixelsPerLine) + posX * poolLineSize;
    }
  }

  void SaveMeta(pugi::xml_node node) {
    ReflectorWrap<IAtlas> wrm(this);
    auto thisNode = ReflectorXMLUtil::SaveV2(wrm, node, true);

    for (auto &i : items) {
      ReflectorWrap<AtlasItem> wr(i.second);
      ReflectorXMLUtil::SaveV2(wr, thisNode, true);
    }
  }

  bool Add(InputItem &item) {
    if (freePools.empty()) {
      return false;
    }

    size_t numPoolsX = item.width / poolPixelsPerLine;
    size_t numPoolsY = item.height / poolPixelsPerLine;

    if (item.width % poolPixelsPerLine) {
      numPoolsX++;
    }

    if (item.height % poolPixelsPerLine) {
      numPoolsY++;
    }

    auto firstFree = freePools.begin();
    uint16 sx = firstFree->first.x;
    uint16 sy = firstFree->first.y;
    uint16 sxEnd = sx + numPoolsX;
    uint16 syEnd = sy + numPoolsY;
    uint16 firstX = sx;
    uint16 firstY = sy;
    bool pass = true;

    for (; sy < syEnd && pass; sy++) {
      for (sx = firstFree->first.x; sx < sxEnd; sx++) {
        if (!freePools.count({sx, sy})) {
          firstX = sx;
          firstY = sy;
          sxEnd = sx + numPoolsX;
          syEnd = sy + numPoolsY;

          if (sxEnd > poolsPerLine || syEnd > poolsPerLine) {
            pass = false;
            break;
          }
        }
      }
    }

    if (!pass) {
      return false;
    }

    AtlasItem nitem;
    nitem.bufferOffset = freePools[{firstX, firstY}];
    nitem.name = std::move(item.name);
    nitem.numX = sxEnd - firstX;
    nitem.numY = syEnd - firstY;
    nitem.X = firstX;
    nitem.Y = firstY;
    items.insert({{firstX, firstY}, std::move(nitem)});

    for (sy = firstY; sy < syEnd; sy++) {
      for (sx = firstX; sx < sxEnd; sx++) {
        freePools.erase({sx, sy});
      }
    }

    const size_t nLineStride = item.width * item.bpp;

    for (size_t h = 0; h < item.height; h++) {
      const size_t bufferOffset = nitem.bufferOffset + h * lineStride;
      memcpy(&buffer[bufferOffset], item.buffer.data() + nLineStride * h,
             nLineStride);
    }

    return true;
  }
};

REFLECTOR_CREATE(IAtlas, 1, VARNAMES, poolsPerLine);

void MakeAtlases(const std::string &path, size_t atlasSize) {
  std::map<DXGI_FORMAT, std::vector<InputItem>> items;
  DirectoryScanner scan;
  scan.AddFilter("dds");
  scan.Scan(path);

  for (auto &s : scan) {
    InputItem it;
    BinReader rd(s);
    DDS header;
    rd.Read(header);
    rd.Seek(header.size + 4);
    if (header.FromLegacy()) {
      throw std::runtime_error("Cannot convert dds to new format!");
    }
    header.ComputeBPP();
    DDS::Mips mips;
    header.ComputeBufferSize(mips);

    if (IsBC(header.dxgiFormat)) {
      it.height = header.height >> 2;
      it.width = header.width >> 2;
      it.bpp = (header.bpp * 16) >> 3;
    } else {
      it.bpp = header.bpp >> 3;
      it.height = header.height;
      it.width = header.width;
    }
    AFileInfo inf(s);
    it.name = inf.GetFilename();
    rd.ReadContainer(it.buffer, mips.sizes[0]);
    items[header.dxgiFormat].emplace_back(std::move(it));
  }

  size_t curAtlas = 0;

  for (auto &i : items) {
    std::sort(i.second.begin(), i.second.end());
    auto &&lastItem = i.second.back();
    size_t atSize = IsBC(i.first) ? atlasSize >> 2 : atlasSize;
    size_t blockSize = std::min(lastItem.width, lastItem.height);
    IAtlas tl(blockSize, atSize, lastItem.bpp);

    for (auto &t : i.second) {
      if (!tl.Add(t)) {
        DDS header;
        header = DDSFormat_DX10;
        header.dxgiFormat = i.first;
        header.size = DDS::DDS_SIZE;

        if (IsBC(header.dxgiFormat)) {
          header.height = atSize << 2;
          header.width = atSize << 2;
        } else {
          header.height = atSize;
          header.width = atSize;
        }

        std::string atlasName = path + "/atlas" + std::to_string(curAtlas);
        BinWritter wr(atlasName + ".dds");
        wr.Write(header);
        wr.WriteContainer(tl.buffer);
        pugi::xml_document doc;
        tl.SaveMeta(doc);
        XMLToFile(atlasName + ".xml", doc);
        tl = IAtlas(blockSize, atSize, lastItem.bpp);
        curAtlas++;
      }
    }
  }
}

#include "datas/pugiex.hpp"
#include "datas/reflector_xml.hpp"
#include "revil/xfs.hpp"

struct XFSClassMember {
  std::string name;
  uint8 type;
  uint8 flags;
  uint16 size;
};

struct XFSClassDesc {
  uint32 hash;
  es::string_view className;
  std::vector<XFSClassMember> members;

  void ToXML(pugi::xml_node node) const;
};

extern std::map<uint32, XFSClassDesc> rttiStore;

struct opt {
  char buffer[0x400]{};

  constexpr opt(es::string_view data) {
    size_t cBuffer = 0;
    for (size_t i = 0; i < data.size(); i++) {
      buffer[cBuffer++] = data[i];
    }
  }
};

#include <array>

std::array<Vector4A16, 6> points{
    Vector4A16{-2, 0, 0, 0}, Vector4A16{3, 0, 0, 0},  Vector4A16{0, -4, 0, 0},
    Vector4A16{0, 5, 0, 0},  Vector4A16{0, 0, -6, 0}, Vector4A16{0, 0, 7, 0},
};

#include "datas/matrix44.hpp"
float covarianceMatrix[4][4]{};
float covarianceMatrix2[4][4]{};
esMatrix44 mtc{{}, {}, {}, {}};

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

esMatrix44 GetCovariance(const std::vector<Vector4A16> &points) {
  esMatrix44 aabb = GetAABB(points);
  auto &means = aabb.r3();
  esMatrix44 retval{{}, {}, {}, means};

  for (int k = 0; k < points.size(); k++) {
    esMatrix44 m1;
    m1.r1() = points[k] - means;
    m1.r2() = m1.r1();
    m1.r3() = m1.r1();
    esMatrix44 m2 = m1;
    m1.Transpose();
    retval.r1() += m1.r1() * m2.r1();
    retval.r2() += m1.r2() * m2.r2();
    retval.r3() += m1.r3() * m2.r3();
  }

  const float frac = 1.f / points.size();
  retval.r1() *= frac;
  retval.r2() *= frac;
  retval.r3() *= frac;

  return retval;
}

void computeCovarianceMatrix() {
  Vector4A16 means;
  for (int i = 0; i < points.size(); i++) {
    means += points[i];
  }

  means /= points.size();

  auto &c = covarianceMatrix;

  for (int k = 0; k < points.size(); k++) {
    esMatrix44 m1;
    m1.r1() = points[k] - means;
    m1.r2() = m1.r1();
    m1.r3() = m1.r1();
    esMatrix44 m2 = m1;
    m1.Transpose();
    mtc.r1() += m1.r1() * m2.r1();
    mtc.r2() += m1.r2() * m2.r2();
    mtc.r3() += m1.r3() * m2.r3();
    /*c[0][0] += (points[k][0] - means[0]) * (points[k][0] - means[0]);
    c[0][1] += (points[k][0] - means[0]) * (points[k][1] - means[1]);
    c[0][2] += (points[k][0] - means[0]) * (points[k][2] - means[2]);

    c[1][0] += (points[k][1] - means[1]) * (points[k][0] - means[0]);
    c[1][1] += (points[k][1] - means[1]) * (points[k][1] - means[1]);
    c[1][2] += (points[k][1] - means[1]) * (points[k][2] - means[2]);

    c[2][0] += (points[k][2] - means[2]) * (points[k][0] - means[0]);
    c[2][1] += (points[k][2] - means[2]) * (points[k][1] - means[1]);
    c[2][2] += (points[k][2] - means[2]) * (points[k][2] - means[2]);
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        covarianceMatrix2[i][j] +=
            (points[k][i] - means[i]) * (points[k][j] - means[j]);
      }
    }*/
  }

  float divisor = points.size() - 1;

  /*for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      c[i][j] /= divisor;
      covarianceMatrix2[i][j] /= divisor;
    }
  }*/

  mtc.r1() /= divisor;
  mtc.r2() /= divisor;
  mtc.r3() /= divisor;
}

static double hypot2(double x, double y) { return sqrt(x * x + y * y); }

// Symmetric Householder reduction to tridiagonal form.

static void tred2(esMatrix44 &V, Vector4A16 &d, Vector4A16 &e) {

  //  This is derived from the Algol procedures tred2 by
  //  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
  //  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
  //  Fortran subroutine in EISPACK.

  for (int j = 0; j < 3; j++) {
    d[j] = V[2][j];
  }

  // Householder reduction to tridiagonal form.

  for (int i = 2; i > 0; i--) {

    // Scale to avoid under/overflow.

    double scale = 0.0;
    double h = 0.0;
    for (int k = 0; k < i; k++) {
      scale = scale + fabs(d[k]);
    }
    if (scale == 0.0) {
      e[i] = d[i - 1];
      for (int j = 0; j < i; j++) {
        d[j] = V[i - 1][j];
        V[i][j] = 0.0;
        V[j][i] = 0.0;
      }
    } else {

      // Generate Householder vector.

      for (int k = 0; k < i; k++) {
        d[k] /= scale;
        h += d[k] * d[k];
      }
      double f = d[i - 1];
      double g = sqrt(h);
      if (f > 0) {
        g = -g;
      }
      e[i] = scale * g;
      h = h - f * g;
      d[i - 1] = f - g;
      for (int j = 0; j < i; j++) {
        e[j] = 0.0;
      }

      // Apply similarity transformation to remaining columns.

      for (int j = 0; j < i; j++) {
        f = d[j];
        V[j][i] = f;
        g = e[j] + V[j][j] * f;
        for (int k = j + 1; k <= i - 1; k++) {
          g += V[k][j] * d[k];
          e[k] += V[k][j] * f;
        }
        e[j] = g;
      }
      f = 0.0;
      for (int j = 0; j < i; j++) {
        e[j] /= h;
        f += e[j] * d[j];
      }
      double hh = f / (h + h);
      for (int j = 0; j < i; j++) {
        e[j] -= hh * d[j];
      }
      for (int j = 0; j < i; j++) {
        f = d[j];
        g = e[j];
        for (int k = j; k <= i - 1; k++) {
          V[k][j] -= (f * e[k] + g * d[k]);
        }
        d[j] = V[i - 1][j];
        V[i][j] = 0.0;
      }
    }
    d[i] = h;
  }

  // Accumulate transformations.

  for (int i = 0; i < 2; i++) {
    V[2][i] = V[i][i];
    V[i][i] = 1.0;
    double h = d[i + 1];
    if (h != 0.0) {
      for (int k = 0; k <= i; k++) {
        d[k] = V[k][i + 1] / h;
      }
      for (int j = 0; j <= i; j++) {
        double g = 0.0;
        for (int k = 0; k <= i; k++) {
          g += V[k][i + 1] * V[k][j];
        }
        for (int k = 0; k <= i; k++) {
          V[k][j] -= g * d[k];
        }
      }
    }
    for (int k = 0; k <= i; k++) {
      V[k][i + 1] = 0.0;
    }
  }
  for (int j = 0; j < 3; j++) {
    d[j] = V[2][j];
    V[2][j] = 0.0;
  }
  V[2][2] = 1.0;
  e[0] = 0.0;
}

// Symmetric tridiagonal QL algorithm.

static void tql2(esMatrix44 &V, Vector4A16 &d, Vector4A16 &e) {

  //  This is derived from the Algol procedures tql2, by
  //  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
  //  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
  //  Fortran subroutine in EISPACK.

  for (int i = 1; i < 3; i++) {
    e[i - 1] = e[i];
  }
  e[2] = 0.0;

  double f = 0.0;
  double tst1 = 0.0;
  double eps = pow(2.0, -52.0);
  for (int l = 0; l < 3; l++) {

    // Find small subdiagonal element

    tst1 = std::max(tst1, fabs(d[l]) + fabs(e[l]));
    int m = l;
    while (m < 3) {
      if (fabs(e[m]) <= eps * tst1) {
        break;
      }
      m++;
    }

    // If m == l, d[l] is an eigenvalue,
    // otherwise, iterate.

    if (m > l) {
      int iter = 0;
      do {
        iter = iter + 1; // (Could check iteration count here.)

        // Compute implicit shift

        double g = d[l];
        double p = (d[l + 1] - g) / (2.0 * e[l]);
        double r = hypot2(p, 1.0);
        if (p < 0) {
          r = -r;
        }
        d[l] = e[l] / (p + r);
        d[l + 1] = e[l] * (p + r);
        double dl1 = d[l + 1];
        double h = g - d[l];
        for (int i = l + 2; i < 3; i++) {
          d[i] -= h;
        }
        f = f + h;

        // Implicit QL transformation.

        p = d[m];
        double c = 1.0;
        double c2 = c;
        double c3 = c;
        double el1 = e[l + 1];
        double s = 0.0;
        double s2 = 0.0;
        for (int i = m - 1; i >= l; i--) {
          c3 = c2;
          c2 = c;
          s2 = s;
          g = c * e[i];
          h = c * p;
          r = hypot2(p, e[i]);
          e[i + 1] = s * r;
          s = e[i] / r;
          c = p / r;
          p = c * d[i] - s * g;
          d[i + 1] = h + s * (c * g + s * d[i]);

          // Accumulate transformation.

          for (int k = 0; k < 3; k++) {
            h = V[k][i + 1];
            V[k][i + 1] = s * V[k][i] + c * h;
            V[k][i] = c * V[k][i] - s * h;
          }
        }
        p = -s * s2 * c3 * el1 * e[l] / dl1;
        e[l] = s * p;
        d[l] = c * p;

        // Check for convergence.

      } while (fabs(e[l]) > eps * tst1);
    }
    d[l] = d[l] + f;
    e[l] = 0.0;
  }

  // Sort eigenvalues and corresponding vectors.

  for (int i = 0; i < 2; i++) {
    int k = i;
    double p = d[i];
    for (int j = i + 1; j < 3; j++) {
      if (d[j] < p) {
        k = j;
        p = d[j];
      }
    }
    if (k != i) {
      d[k] = d[i];
      d[i] = p;
      for (int j = 0; j < 3; j++) {
        p = V[j][i];
        V[j][i] = V[j][k];
        V[j][k] = p;
      }
    }
  }
}

// rows 0 - 2: eigenvectors
// row3: eigenvalues
esMatrix44 eigen_decomposition(esMatrix44 in) {
  Vector4A16 e;
  tred2(in, in.r4(), e);
  tql2(in, in.r4(), e);
  return in;
}

struct OOBBResult {
  esMatrix44 transform;
  Vector4A16 min;
  Vector4A16 max;
};

OOBBResult GetOOBB(std::vector<Vector4A16> &points) {
  auto covMatrix = GetCovariance(points);
  OOBBResult retval;
  retval.transform = eigen_decomposition(covMatrix);
  retval.transform.r4() = -covMatrix.r4();

  for (auto &p : points) {
    p = retval.transform.RotatePoint(p);
  }

  auto localAABB = GetAABB(points);
  retval.transform.r4() *= -1;
  retval.max = localAABB.r1();
  retval.min = localAABB.r2();
  return retval;
}

#include "gltf.h"
#include "revil/mod.hpp"
#include "uni/model.hpp"
#include "uni/rts.hpp"
#include "uni/skeleton.hpp"
#include <set>

using namespace fx;

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

int main() {
  printer.AddPrinterFunction(
      reinterpret_cast<MasterPrinterThread::print_func>(printf));

  constexpr opt imizer({"some data lolol", 16});

  DirectoryScanner scan;
  /*scan.AddFilter(".max");
  scan.Scan("/mnt/p/PROJEKTY/dragona/_NPC");
  for (auto &f : scan) {
    printline(f);
  }*/

  scan.AddFilter(".dom");
  scan.AddFilter(".mod");
  scan.Scan("/mnt/r/Unpacked Games/s921/allmod/dr_x360/");

  for (auto &f : scan) {
    es::string_view sw(f);
    // printf("%s\n", sw.data());
    revil::MOD mod;
    // try {
    mod.Load(f);
    GLTF main;
    main.Pipeline(mod);
    AFileInfo info(f);
    auto outFile = info.GetFullPathNoExt().to_string() + (".glb");
    gltf::Save(main.doc, outFile, true);
    /*} catch (const std::exception &e) {
      printline(f);
      printline(e.what());
    }*/
  }

  return 0;

  auto fileName = "/mnt/r/Unpacked "
                  //"Games/s921/allmod/dr_x360/model/om/om0123/om0123.dom";
                  //"Games/s921/allmod/dr_x360/scroll/stageb/sb03/sb03_00.dom";
                  "Games/s921/allmod/dr_x360/model/em/em02/em02.dom";
  //"Games/s921/allmod/dr_x360/effect/efm/efm0056/efm0056.dom";

  revil::MOD mod;
  mod.Load(fileName);

  GLTF main;
  main.Pipeline(mod);
  AFileInfo info(fileName);
  auto outFile = info.GetFullPathNoExt().to_string() + (".glb");
  gltf::Save(main.doc, outFile, true);
  return 0;
  esMatrix44 mtq;
  mtq.FromQuat(Vector4A16(0.7, 0, 0.7, 0));

  for (auto &p : points) {
    p = mtq.RotatePoint(p);
  }

  computeCovarianceMatrix();
  esMatrix44 mtx;
  memcpy(&mtx, covarianceMatrix, sizeof(mtx));
  esMatrix44 mt2;
  memcpy(&mt2, covarianceMatrix2, sizeof(mtx));
  auto &mt23 = mtc;
  auto er = eigen_decomposition(mtc);
  auto dp = er.r1().Dot(er.r2());
  er.r1().Normalize();
  er.r2().Normalize();
  er.r3() = er.r1().Cross(er.r2());
  auto qt = er.ToQuat();

  return 0;

  // Decompress("/mnt/r/Unpacked Games/s921/s921.ori");
  // Decompress("/mnt/r/Unpacked Games/s921/ma0_s600.ori");

  // Decompress("/mnt/r/Unpacked
  // Games/rer/dat/DAT_00/dlc/archive/game/chara/pl/pl11a0/pl11a0.arc");

  /*const uint32 vertexID = 0;
  // 2016931628 ARMCO_LODA12
  // 3615898766 armco_GP_loda01

  // 2498562874 ARMCO_LODA12
  // 224929464, 3386432508 armco_GP_loda01

  for (size_t i = 0; i <= 0xffffffff; i++) {
    auto ret = sub_140AD02E0("ARMCO_LODA12", 12, i, 0);
    if (ret == 2016931628) {
      printline(i);
    }
  }

  auto doStuff = [](auto &&callBack) { callBack(1, 2, 3); };

  auto cb = [](int i, float f, bool b) {};

  auto fnc = [&](auto &&callBack) {
    doStuff([&](auto&&... stuff) { callBack(stuff...); });
  };

  fnc(cb);

  for (size_t i = 0; i <= 0xffffffff; i++) {
    uint32 shiftFactor = (-(vertexID & 31) + 32) & 31;
    uint32 encKey = vertexID ^ i;
    encKey ^= 0x6ed5dead;
    uint32 encKey2 = encKey >> shiftFactor;
    encKey <<= vertexID & 31;
    uint32 finalKey = encKey2 | encKey;

    if ((finalKey ^ 3237300176) == 3591861853) {
      printline(i);
    }
  }
  return 0;*/

  /*IAtlas tl(128, 512, 8);

  MakeAtlases("/mnt/r/Unpacked Games/s921/_/", 4096);

  scan.AddFilter("tex");
  scan.Scan("/mnt/r/Unpacked Games/s921/alltex/lp2");

  RunThreadedQueue(scan.Files().size(), [&](size_t index) {
    es::string_view f = scan.Files()[index];
    try {
      BinReader rd(f);
      AFileInfo info(f);
      TEX tx = TEX::Create(rd);
      auto outFileName = info.GetFullPathNoExt().to_string() + ".dds";
      BinWritter wr(outFileName);
      Tex2DdsSettings set;
      set.convertIntoLegacy = true;
      set.convertIntoLegacyNonCannon = true;
      set.noMips = true;
      tx.SaveAsDDS(wr, set);
    } catch (std::exception &e) {
      printerror(f << ' ' << e.what());
    }
  });

  return 0;*/

  scan.AddFilter("486");
  scan.Scan("/mnt/r/Unpacked Games/re engine/reverse");

  for (auto &f : scan) {
    printline(f);
    revil::REAsset reAsset;
    reAsset.Load(f);
  }
  return 0;

  /* scan.AddFilter("mod");
   scan.Scan("/mnt/r/Unpacked Games/s921/hd2c00");
   // scan.Scan("/mnt/r/Unpacked Games/s921/allmod/lp2");

   for (auto &f : scan) {
     LoadMOD(f);
   }

   CompressionSettings set;
   set.uncompZLIBHeader = true;
   set.verbose = true;
   set.extensions = &extLP2;
   set.platform = ARCPlatform::WinPC;

   Compress("/mnt/r/Unpacked Games/s921/hd2c00", set);*/
  // Compress("/mnt/r/Unpacked Games/s921/s921");

  /* scan.Scan("/mnt/r/Unpacked Games/s921/alldata/lp_ps3/tool");

   for (auto &fl : scan.Files()) {
     BinReader rd(fl);
     es::string_view sw(fl);
     try {
       XFS xfs;
       xfs.Load(rd);
       printf("Load: %s\n", fl.data());
       pugi::xml_document doc;
       xfs.ToXML(doc);
       // XMLToFile(fl + ".xml", doc);

     } catch (es::InvalidHeaderError &) {
     }
   }

   pugi::xml_document doc;
   auto rNode = doc.append_child("RTTI");

   for (auto &c : rttiStore) {
     c.second.ToXML(rNode);
   }

   XMLToFile("dump.xml", doc);*/

  return 0;
}
