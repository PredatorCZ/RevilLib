#include "datas/binreader.hpp"
#include "datas/binwritter.hpp"
#include "datas/directory_scanner.hpp"
#include "datas/fileinfo.hpp"
#include "datas/master_printer.hpp"
#include "datas/multi_thread.hpp"
#include "revil/re_asset.hpp"
#include <vector>

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

int main() {
  printer.AddPrinterFunction(
      reinterpret_cast<MasterPrinterThread::print_func>(printf));
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



  IAtlas tl(128, 512, 8);

  MakeAtlases("/mnt/r/Unpacked Games/s921/_/", 4096);

  return 0;
}
