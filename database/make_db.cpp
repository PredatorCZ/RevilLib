#include "database.hpp"
#include "revil/hashreg.hpp"
#include "spike/io/binwritter.hpp"
#include "spike/io/directory_scanner.hpp"
#include "spike/reflect/reflector.hpp"
#include <algorithm>
#include <array>
#include <functional>
#include <set>
#include <sstream>

MAKE_ENUM(ENUMSCOPE(class revil__Family, revil__Family), EMEMBER(NONE),
          EMEMBER(Classes), EMEMBER(ARC), EMEMBER(MOD), EMEMBER(LMT),
          EMEMBER(TEX), EMEMBER(XFS));

MAKE_ENUM(ENUMSCOPE(class revil__Platform, revil__Platform), EMEMBER(Auto),
          EMEMBER(Win32), EMEMBER(N3DS), EMEMBER(PS3), EMEMBER(X360),
          EMEMBER(CAFE), EMEMBER(NSW), EMEMBER(PS4), EMEMBER(Android),
          EMEMBER(IOS), EMEMBER(Win64));
using namespace revil;

using Executor = void (*)(revil__Platform, std::string_view, std::string_view);

struct DbClass {
  std::string name;
  std::string extension;
};

struct ArcSupport {
  uint8 version = 7;
  uint8 windowSize = 15;
  bool allowRaw = false;
  bool xmemOnly = false;
  bool extendedFilePath = false;
  std::string key{};
};

static constexpr ArcSupport ARC_PS3_GENERIC{8, 14, true};
static constexpr ArcSupport ARC_WINPC_GENERIC{};
static constexpr ArcSupport ARC_N3DS_GENERIC{0x11};

struct TitleDB {
  bool version1 = false;
  std::vector<std::string> titles;
  std::vector<DbClass> classes[NUM_PLATFORMS];
  std::vector<std::string> otherClasses;
  ArcSupport arcSuport[NUM_PLATFORMS]{
      {0},
      ARC_WINPC_GENERIC,
      ARC_N3DS_GENERIC,
      ARC_PS3_GENERIC,
      {},
      {},
      {},
      {},
      {},
      {},
      {},
  };
  uint16 modVersions[NUM_PLATFORMS]{};
  uint16 lmtVersions[NUM_PLATFORMS]{};
  uint16 texVersions[NUM_PLATFORMS]{};
  uint16 xfsVersions[NUM_PLATFORMS]{};
  bool usedPlatforms[NUM_PLATFORMS]{};
};

static std::vector<TitleDB> GLOBAL_DB;

uint32 GetNumber(std::string_view value) {
  return strtoul(value.data(), nullptr, 10 + 6 * value.starts_with("0x"));
}

static const Executor executors[]{
    [](revil__Platform, std::string_view key, std::string_view value) {
      if (key == "Version") {
        GLOBAL_DB.back().version1 = GetNumber(value) == 1;
      } else if (key == "Titles") {
        while (value.size() > 0) {
          const size_t found = value.find(',');

          GLOBAL_DB.back().titles.emplace_back(
              es::TrimWhitespace(value.substr(0, found)));

          if (found == value.npos) {
            break;
          }

          value.remove_prefix(found + 1);
          value = es::TrimWhitespace(value);
        }
      }
    },
    [](revil__Platform plt, std::string_view key, std::string_view value) {
      if (value.empty()) {
        GLOBAL_DB.back().otherClasses.emplace_back(key);
        return;
      }

      DbClass cls;
      cls.name = key;
      cls.extension = value;
      GLOBAL_DB.back().classes[uint32(plt)].emplace_back(cls);
    },
    [](revil__Platform plt, std::string_view key, std::string_view value) {
      auto &arc = GLOBAL_DB.back().arcSuport[uint32(plt)];
      if (key == "Version") {
        arc.version = GetNumber(value);
      } else if (key == "XMemdecompress") {
        arc.xmemOnly = true;
      } else if (key == "AllowRaw") {
        arc.allowRaw = true;
      } else if (key == "ExtendedPath") {
        arc.extendedFilePath = true;
      } else if (key == "WindowSize") {
        arc.windowSize = GetNumber(value);
      } else if (key == "Key") {
        arc.key = value;
      }
    },
    [](revil__Platform plt, std::string_view key, std::string_view value) {
      auto &mod = GLOBAL_DB.back().modVersions[uint32(plt)];
      if (key == "Version") {
        mod = GetNumber(value);
      }
    },
    [](revil__Platform plt, std::string_view key, std::string_view value) {
      auto &lmt = GLOBAL_DB.back().lmtVersions[uint32(plt)];
      if (key == "Version") {
        lmt = GetNumber(value);
      }
    },
    [](revil__Platform plt, std::string_view key, std::string_view value) {
      auto &tex = GLOBAL_DB.back().texVersions[uint32(plt)];
      if (key == "Version") {
        tex = GetNumber(value);
      }
    },
    [](revil__Platform plt, std::string_view key, std::string_view value) {
      auto &xfs = GLOBAL_DB.back().xfsVersions[uint32(plt)];
      if (key == "Version") {
        xfs = GetNumber(value);
      }
    },
};

struct StringSlider {
  std::string buffer;

  std::string::iterator FindString(std::string_view str) {
    if (str.size() > buffer.size()) [[unlikely]] {
      if (std::string_view(str).starts_with(buffer)) [[unlikely]] {
        buffer = str;
        return buffer.begin();
      } else {
        return buffer.end();
      }
    }
    auto searcher = std::boyer_moore_horspool_searcher(str.begin(), str.end());
    return std::search(buffer.begin(), buffer.end(), searcher);
  }

  size_t InsertString(std::string_view str) {
    auto found = FindString(str);

    if (found == buffer.end()) {
      buffer.append(str.data(), str.size());
      return buffer.size() - str.size();
    } else {
      return std::distance(buffer.begin(), found);
    }
  }
};

void LoadDb() {
  DirectoryScanner sc;
  sc.Scan("database/data");

  for (auto &f : sc) {
    std::ifstream stream(f);
    char buffer[0x100];
    size_t lineNum = 0;
    revil__Family curFamily = revil__Family::NONE;
    revil__Platform curPlatform = revil__Platform::Auto;
    GLOBAL_DB.emplace_back();
    bool usedArcs[NUM_PLATFORMS]{};

    while (!stream.eof()) {
      stream.getline(buffer, sizeof(buffer));
      lineNum++;
      std::string_view sv(buffer);
      es::TrimWhitespace(sv);

      if (sv.empty()) {
        continue;
      }

      if (sv.front() == '[') {
        if (sv.back() != ']') {
          throw std::runtime_error(f + ":" + std::to_string(lineNum) +
                                   ": expedted ] at the end");
        }

        sv.remove_prefix(1);
        sv.remove_suffix(1);

        const size_t foundSep = sv.find(':');
        std::string_view familyName(sv.substr(0, foundSep));
        static const auto familyRef = GetReflectedEnum<revil__Family>();
        bool foundFamily = false;

        for (size_t e = 0; e < familyRef->numMembers; e++) {
          if (familyRef->names[e] == familyName) {
            curFamily = static_cast<revil__Family>(e);
            curPlatform = revil__Platform::Auto;
            foundFamily = true;
            break;
          }
        }

        if (!foundFamily) {
          throw std::runtime_error(
              f + ":" + std::to_string(lineNum) +
              ": cannot find family: " + std::string(familyName));
        }

        if (foundSep != sv.npos) {
          std::string_view platform(sv.substr(foundSep + 1));

          if (platform.empty()) {
            throw std::runtime_error(f + ":" + std::to_string(lineNum) +
                                     ": expected platform name after :");
          }

          static const auto refPlatform = GetReflectedEnum<revil__Platform>();
          bool foundPlatform = false;

          for (size_t e = 0; e < refPlatform->numMembers; e++) {
            if (refPlatform->names[e] == platform) {
              curPlatform = static_cast<revil__Platform>(e);
              GLOBAL_DB.back().usedPlatforms[e] = true;
              foundPlatform = true;
              break;
            }
          }

          if (!foundPlatform) {
            throw std::runtime_error(
                f + ":" + std::to_string(lineNum) +
                ": cannot find platform: " + std::string(platform));
          }
        }

        continue;
      } else if (sv.back() == ']') {
        throw std::runtime_error(f + ":" + std::to_string(lineNum) +
                                 ": expedted [ at the start");
      }

      const size_t foundEq = sv.find('=');

      std::string_view key(es::TrimWhitespace(sv.substr(0, foundEq)));
      std::string_view value;

      if (foundEq != sv.npos) {
        value = es::TrimWhitespace(sv.substr(foundEq + 1));

        if (value.empty()) {
          throw std::runtime_error(f + ":" + std::to_string(lineNum) +
                                   ": expedted value after =");
        }
      }

      executors[uint32(curFamily)](curPlatform, key, value);

      if (curFamily == revil__Family::ARC) {
        usedArcs[uint32(curPlatform)] = true;
      }
    }

    if (usedArcs[0]) {
      for (uint32 p = 1; p < NUM_PLATFORMS; p++) {
        if (!usedArcs[p] && GLOBAL_DB.back().usedPlatforms[p]) {
          GLOBAL_DB.back().arcSuport[p] = GLOBAL_DB.back().arcSuport[0];
        }
      }
    }
  }
}

using CompareString = decltype([](auto &i1, auto &i2) {
  if (i1.size() == i2.size()) {
    return i1 < i2;
  }

  return i1.size() > i2.size();
});

auto DoStrings(BinWritterRef wr) {
  std::map<std::string_view, uint32> sliderOffsets;
  std::set<std::string_view, CompareString> strings;

  for (auto &d : GLOBAL_DB) {
    for (auto &t : d.titles) {
      strings.emplace(t);
    }

    for (auto &c : d.classes) {
      for (auto &i : c) {
        strings.emplace(i.name);

        if (i.extension.size() > 0) {
          strings.emplace(i.extension);
        }
      }
    }

    for (auto &c : d.otherClasses) {
      strings.emplace(c);
    }
  }

  StringSlider slider;
  const uint32 stringsBegin = wr.Tell();

  for (auto s : strings) {
    sliderOffsets.emplace(s, slider.InsertString(s) + stringsBegin);
  }

  for (auto &d : GLOBAL_DB) {
    for (uint32 p = 0; p < NUM_PLATFORMS; p++) {
      auto &key = d.arcSuport[p].key;
      if (key.size() > 0) {
        sliderOffsets.emplace(key, slider.InsertString(key) + stringsBegin);
      }
    }
  }

  wr.WriteContainer(slider.buffer);
  return sliderOffsets;
}

Header DoClasses(BinWritterRef wr,
                 const std::map<std::string_view, uint32> &sliderOffsets) {
  struct ExtInfo {
    std::map<uint32, uint32> platformWeights;
  };

  struct ClassInfo {
    std::map<std::string_view, ExtInfo> extensions;
    bool version21[2]{};
  };

  struct UClassInfo {
    bool version21[2]{};
  };

  std::map<std::string_view, ClassInfo> sortedClasses;
  std::map<std::string_view, UClassInfo> sortedUClasses;
  std::map<std::string_view, std::set<std::string_view>, CompareString>
      sortedExtensions;

  for (auto &d : GLOBAL_DB) {
    uint32 curPlatform = 0;
    for (auto &c : d.classes) {
      for (auto &i : c) {
        sortedClasses[i.name].version21[d.version1] = true;

        if (i.extension.size() > 0) {
          sortedClasses[i.name]
              .extensions[i.extension]
              .platformWeights[curPlatform]++;

          sortedExtensions[i.extension].emplace(i.name);
        }
      }

      curPlatform++;
    }

    for (auto &c : d.otherClasses) {
      sortedUClasses[c].version21[d.version1] = true;
    }
  }

  wr.ApplyPadding(4);
  std::vector<ResourceClass> rClasses;

  auto NewResClass = [&](auto &c, bool v1Hash) {
    auto &res = rClasses.emplace_back();

    if (v1Hash) {
      res.hash = MTHashV1(c.first);
    } else {
      res.hash = MTHashV2(c.first);
    }

    res.name.offset = sliderOffsets.at(c.first);
    res.name.size = c.first.size();

    if (c.second.extensions.size() == 1) {
      res.extension.offset =
          sliderOffsets.at(c.second.extensions.begin()->first);
      res.extension.size = c.second.extensions.begin()->first.size();
    } else {
      std::string_view mostAuto;
      size_t maxWeightAuto = 0;
      std::string_view mostOther;
      size_t maxWeightOther = 0;

      for (auto &[ex, plt] : c.second.extensions) {
        for (auto &[p, w] : plt.platformWeights) {
          if (p == 0) {
            if (w > maxWeightAuto) {
              maxWeightAuto = w;
              mostAuto = ex;
            }
          } else if (w > maxWeightOther) {
            maxWeightOther = w;
            mostOther = ex;
          }
        }
      }

      if (maxWeightAuto > 0) {
        res.extension.offset = sliderOffsets.at(mostAuto);
        res.extension.size = mostAuto.size();
      } else {
        res.extension.offset = sliderOffsets.at(mostOther);
        res.extension.size = mostOther.size();
      }
    }
  };

  for (auto &c : sortedClasses) {
    if (c.second.extensions.size() > 0) {

      if (c.second.version21[0]) {
        NewResClass(c, false);
      }

      if (c.second.version21[1]) {
        NewResClass(c, true);
      }
    }
  }
  auto WriteClasses = [&] {
    std::sort(rClasses.begin(), rClasses.end(),
              [](auto &c1, auto &c2) { return c1.hash < c2.hash; });

    for (auto &c : rClasses) {
      const int32 namePtrOffset = offsetof(ResourceClass, name) + wr.Tell();
      c.name.offset -= namePtrOffset;
      const int32 extPtrOffset = offsetof(ResourceClass, extension) + wr.Tell();
      c.extension.offset -= extPtrOffset;

      wr.Write(c);
    }

    es::Dispose(rClasses);
  };

  Header hdr{};
  hdr.resourceClasses[0].data.varPtr = wr.Tell() - 8;
  hdr.resourceClasses[0].numItems = rClasses.size();
  WriteClasses();

  auto NewResClassPlt = [&](auto &c, bool v1Hash, uint32 platform) {
    std::string_view mostAuto;
    size_t maxWeightAuto = 0;
    std::string_view mostOther;
    size_t maxWeightOther = 0;

    for (auto &[ex, plt] : c.second.extensions) {
      for (auto &[p, w] : plt.platformWeights) {
        if (p == 0) {
          if (w > maxWeightAuto) {
            maxWeightAuto = w;
            mostAuto = ex;
          }
        } else if (w > maxWeightOther) {
          maxWeightOther = w;
          mostOther = ex;
        }
      }
    }

    std::string_view mostExt;
    size_t maxWeight = 0;

    for (auto &[ex, plt] : c.second.extensions) {
      for (auto &[p, w] : plt.platformWeights) {
        if (p == platform) {
          if (w > maxWeight) {
            maxWeight = w;
            mostExt = ex;
          }
        }
      }
    }

    if (mostExt.empty() || (mostAuto.empty() && mostExt == mostOther)) {
      return;
    }

    auto &res = rClasses.emplace_back();

    if (v1Hash) {
      res.hash = MTHashV1(c.first);
    } else {
      res.hash = MTHashV2(c.first);
    }

    res.name.offset = sliderOffsets.at(c.first);
    res.name.size = c.first.size();

    res.extension.offset = sliderOffsets.at(mostExt);
    res.extension.size = mostExt.size();
  };

  for (uint32 p = 1; p < NUM_PLATFORMS; p++) {
    for (auto &c : sortedClasses) {
      auto &exts = c.second.extensions;

      if (exts.size() > 1) {
        if (c.second.version21[0]) {
          NewResClassPlt(c, false, p);
        }

        if (c.second.version21[1]) {
          NewResClassPlt(c, true, p);
        }
      }
    }

    hdr.resourceClasses[p].data.varPtr = wr.Tell() - (8 * p) - 8;
    hdr.resourceClasses[p].numItems = rClasses.size();
    WriteClasses();
  }

  es::Dispose(rClasses);

  std::vector<Class> uClasses;

  for (auto &[name, info] : sortedUClasses) {
    if (info.version21[0]) {
      auto &res = uClasses.emplace_back();
      res.hash = MTHashV2(name);
      res.name = {{
          .size = uint32(name.size()),
          .offset = int32(sliderOffsets.at(name)),
      }};
    }

    if (info.version21[1]) {
      auto &res = uClasses.emplace_back();
      res.hash = MTHashV1(name);
      res.name = {{
          .size = uint32(name.size()),
          .offset = int32(sliderOffsets.at(name)),
      }};
    }
  }

  std::sort(uClasses.begin(), uClasses.end(),
            [](auto &c1, auto &c2) { return c1.hash < c2.hash; });

  hdr.classes.data.varPtr = wr.Tell() - offsetof(Header, classes.data);
  hdr.classes.numItems = uClasses.size();

  for (auto &c : uClasses) {
    const int32 namePtrOffset = offsetof(Class, name) + wr.Tell();
    c.name.offset -= namePtrOffset;
    wr.Write(c);
  }

  es::Dispose(uClasses);
  // std::map<std::string_view, uint8, CompareString> extNs;
  std::set<std::string_view, CompareString> extNs;

  for (uint32 v = 0; v < 2; v++) {
    hdr.extensions[v].data.varPtr =
        wr.Tell() -
        (offsetof(Header, extensions[0].data) + sizeof(Array<Extension>) * v);

    for (auto &[ext, cls] : sortedExtensions) {
      std::vector<std::string_view> classes;

      for (auto &c : cls) {
        auto &cInfo = sortedClasses.at(c);
        if (cInfo.version21[v]) {
          classes.emplace_back(c);
        }
      }

      if (classes.size() != 1) {
        continue;
      }

      hdr.extensions[v].numItems++;

      wr.Write(Extension{
          .extension{{
              .size = uint32(ext.size()),
              .offset = int32(sliderOffsets.at(ext) - wr.Tell()),
          }},
          .hash = v ? MTHashV1(classes.front()) : MTHashV2(classes.front()),
      });
    }

    hdr.extensions4[v].data.varPtr =
        wr.Tell() -
        (offsetof(Header, extensions4[0].data) + sizeof(Array<Extension>) * v);

    for (auto &[ext, cls] : sortedExtensions) {
      std::vector<std::string_view> classes;

      for (auto &c : cls) {
        auto &cInfo = sortedClasses.at(c);
        if (cInfo.version21[v]) {
          classes.emplace_back(c);
        }
      }

      if (classes.size() < 2) {
        continue;
      }

      if (classes.size() > 4) {
        if (v == 0) {
          extNs.emplace(ext);
        } else {
          throw std::runtime_error("ExtensionN not implemented for version 1");
        }

        // extNs[ext] |= 1 << v;
        continue;
      }

      hdr.extensions4[v].numItems++;

      Extension4 ext4{
          .extension{{
              .size = uint32(ext.size()),
              .offset = int32(sliderOffsets.at(ext) - wr.Tell()),
          }},
          .hashes{},
      };

      for (size_t c = 0; c < classes.size(); c++) {
        ext4.hashes[c] = v ? MTHashV1(classes.at(c)) : MTHashV2(classes.at(c));
      }

      wr.Write(ext4);
    }
  }

  std::vector<SmallArray<uint32>> hashesN;

  /*for (auto &[ext, versions] : extNs) {
    SmallArray<uint32> curItem{
        .size = 0,
        .offset = int32(wr.Tell()),
    };

    auto &cls = sortedExtensions.at(ext);

    for (auto &c : cls) {
      auto &cInfo = sortedClasses.at(c);
      for (uint32 v = 0; v < 2; v++) {
        if ((versions & (1 << v)) && cInfo.version21[v]) {
          wr.Write(v ? MTHashV1(c) : MTHashV2(c));
          curItem.size++;
        }
      }
    }

    hashesN.emplace_back(curItem);
  }*/

  for (auto ext : extNs) {
    SmallArray<uint32> curItem{
        .size = 0,
        .offset = int32(wr.Tell()),
    };

    auto &cls = sortedExtensions.at(ext);

    for (auto &c : cls) {
      wr.Write(MTHashV2(c));
      curItem.size++;
    }

    hashesN.emplace_back(curItem);
  }

  hdr.extensionsN.data.varPtr = wr.Tell() - offsetof(Header, extensionsN.data);
  hdr.extensionsN.numItems = hashesN.size();

  for (auto keys = extNs.begin(); auto &arr : hashesN) {
    ExtensionN ext{
        .extension{{
            .size = uint32(keys->size()),
            .offset = int32(sliderOffsets.at(*keys) - wr.Tell()),
        }},
        .hashes = arr,
    };
    ext.hashes.offset -= wr.Tell() + offsetof(ExtensionN, hashes);
    wr.Write(ext);

    keys++;
  }

  return hdr;
}

size_t DoSupport(BinWritterRef wr,
                 const std::map<std::string_view, uint32> &sliderOffsets) {
  std::map<TitleSupport, uint32> supportPalette;

  auto DbArc = [&sliderOffsets](auto &t, uint32 p) {
    DbArcSupport arc{
        .version = t.arcSuport[p].version,
        .windowSize = t.arcSuport[p].windowSize,
        .flags =
            uint8(t.arcSuport[p].allowRaw |
                  (int(t.arcSuport[p].extendedFilePath) << 1) |
                  (int(t.arcSuport[p].xmemOnly) << 2) | (int(t.version1) << 3)),
        .key = {},
    };

    if (auto &key = t.arcSuport[p].key; key.size() > 0) {
      arc.key = {{
          .size = uint8(key.size()),
          .offset = int32(sliderOffsets.at(key)),
      }};
    }

    return arc;
  };

  std::vector<std::array<int32, NUM_PLATFORMS - 1>> indicesPalette;

  for (auto &t : GLOBAL_DB) {
    std::array<int32, NUM_PLATFORMS - 1> indices;

    for (uint32 p = 1; p < NUM_PLATFORMS; p++) {
      if (!t.usedPlatforms[p]) {
        indices[p - 1] = -1;
        continue;
      }

      TitleSupport supp{
          .arc = t.arcSuport[p].version ? DbArc(t, p) : DbArc(t, 0),
          .modVersion = t.modVersions[p] ? t.modVersions[p] : t.modVersions[0],
          .lmtVersion = t.lmtVersions[p] ? t.lmtVersions[p] : t.lmtVersions[0],
          .texVersion = t.texVersions[p] ? t.texVersions[p] : t.texVersions[0],
          .xfsVersion = t.xfsVersions[p] ? t.xfsVersions[p] : t.xfsVersions[0],
      };

      if (auto found = supportPalette.find(supp);
          found == supportPalette.end()) {
        indices[p - 1] = supportPalette.size();
        supportPalette.emplace(supp, supportPalette.size());
      } else {
        indices[p - 1] = found->second;
      }
    }

    indicesPalette.emplace_back(indices);
  }

  const size_t supportPaletteBegin = wr.Tell();

  {
    std::vector<TitleSupport> sortedSupports;
    sortedSupports.resize(supportPalette.size());

    for (auto &[t, i] : supportPalette) {
      sortedSupports.at(i) = t;
    }

    for (auto &s : sortedSupports) {
      if (s.arc.key.size > 0) {
        s.arc.key.offset -= wr.Tell() + offsetof(TitleSupport, arc.key);
      }

      wr.Write(s);
    }
  }

  const size_t indicesBegin = wr.Tell();

  for (auto i : indicesPalette) {
    for (auto &t : i) {
      if (t < 0) {
        t = 0;
      } else {
        t = (supportPaletteBegin + t * sizeof(TitleSupport)) - wr.Tell();
      }
      wr.Write(t);
    }
  }

  return indicesBegin;
}

std::vector<size_t>
DoTitleFixups(BinWritterRef wr, size_t indicesBegin, const Header *hdr,
              const std::map<std::string_view, uint32> &sliderOffsets) {
  std::vector<size_t> titleOffsets;
  auto &aClasses = hdr->resourceClasses[0];
  int32 curTitle = -1;

  for (auto &d : GLOBAL_DB) {
    curTitle++;
    std::map<uint32, std::vector<ResourceClass>> fixups;

    for (uint32 p = 0; p < NUM_PLATFORMS; p++) {
      if (!d.usedPlatforms[p] && p > 0) {
        continue;
      }

      auto &sClasses = hdr->resourceClasses[p];

      for (auto &c : d.classes[p]) {
        const uint32 hash = d.version1 ? MTHashV1(c.name) : MTHashV2(c.name);

        auto found = std::lower_bound(sClasses.begin(), sClasses.end(), hash);

        if (found == sClasses.end() || found->hash != hash) {
          found = std::lower_bound(aClasses.begin(), aClasses.end(), hash);

          if (found == aClasses.end()) {
            throw std::runtime_error("Cannot find class");
          }
        }

        if (found->hash != hash) {
          throw std::runtime_error("Cannot find class");
        }

        if (std::string_view(found->extension) != c.extension) {
          fixups[p].emplace_back(ResourceClass{
              Class{
                  .hash = hash,
                  .name = {{
                      .size = uint32(c.name.size()),
                      .offset = int32(sliderOffsets.at(c.name)),
                  }},
              },
              {{
                  .size = uint32(c.extension.size()),
                  .offset = int32(sliderOffsets.at(c.extension)),
              }},
          });
        }
      }
    }

    std::vector<Fixup> platformFixups;

    for (auto &[plt, fix] : fixups) {
      std::sort(fix.begin(), fix.end(),
                [](auto &c1, auto &c2) { return c1.hash < c2.hash; });
      platformFixups.emplace_back(Fixup{
          .classes{.varPtr = int32(wr.Tell())},
          .platform = uint8(plt),
          .numItems = uint16(fix.size()),
      });

      for (auto &c : fix) {
        const int32 namePtrOffset = offsetof(ResourceClass, name) + wr.Tell();
        c.name.offset -= namePtrOffset;
        const int32 extPtrOffset =
            offsetof(ResourceClass, extension) + wr.Tell();
        c.extension.offset -= extPtrOffset;

        wr.Write(c);
      }
    }

    const int32 fixupsBegin = wr.Tell();

    for (auto &f : platformFixups) {
      f.classes.varPtr -= wr.Tell();
      wr.Write(f);
    }

    Title title{
        .support{.varPtr = int32(indicesBegin +
                                 curTitle * sizeof(int32[NUM_PLATFORMS - 1])) -
                           int32(wr.Tell() - offsetof(Title, support))},
        .fixups{.size = uint32(platformFixups.size()),
                .offset =
                    (platformFixups.size() > 0) *
                    int32(fixupsBegin - wr.Tell() - offsetof(Title, fixups))},
    };

    titleOffsets.emplace_back(wr.Tell());
    wr.Write(title);
  }

  return titleOffsets;
}

void ValidateDb(const Header *shdr) {
  for (auto &d : GLOBAL_DB) {
    for (auto &t : d.titles) {
      auto oKey = LowerBound(t, shdr->titles);

      if (oKey == shdr->titles.end() || std::string_view(oKey->name) != t) {
        throw std::runtime_error("Cannot find title");
      }

      const Title &title = oKey->data;

      auto supports = title.support.operator->();

      for (size_t p = 1; p < NUM_PLATFORMS; p++) {
        if (!d.usedPlatforms[p]) {
          if (supports[p - 1].operator->()) {
            throw std::runtime_error("Validation failed");
          }

          continue;
        }

        const TitleSupport &support = supports[p - 1];

        if (support.arc.version != d.arcSuport[p].version) {
          throw std::runtime_error("Validation failed");
        }

        if (support.arc.windowSize != d.arcSuport[p].windowSize) {
          throw std::runtime_error("Validation failed");
        }

        if (std::string_view(support.arc.key) != d.arcSuport[p].key) {
          throw std::runtime_error("Validation failed");
        }

        if (bool(support.arc.flags & DbArc_AllowRaw) !=
            d.arcSuport[p].allowRaw) {
          throw std::runtime_error("Validation failed");
        }

        if (bool(support.arc.flags & DbArc_ExtendedPath) !=
            d.arcSuport[p].extendedFilePath) {
          throw std::runtime_error("Validation failed");
        }

        if (bool(support.arc.flags & DbArc_XMemCompress) !=
            d.arcSuport[p].xmemOnly) {
          throw std::runtime_error("Validation failed");
        }

        if (bool(support.arc.flags & DbArc_Version1) != d.version1) {
          throw std::runtime_error("Validation failed");
        }

        if (support.lmtVersion !=
            (d.lmtVersions[p] ? d.lmtVersions[p] : d.lmtVersions[0])) {
          throw std::runtime_error("Validation failed");
        }

        if (support.modVersion !=
            (d.modVersions[p] ? d.modVersions[p] : d.modVersions[0])) {
          throw std::runtime_error("Validation failed");
        }

        if (support.texVersion !=
            (d.texVersions[p] ? d.texVersions[p] : d.texVersions[0])) {
          throw std::runtime_error("Validation failed");
        }

        if (support.xfsVersion !=
            (d.xfsVersions[p] ? d.xfsVersions[p] : d.xfsVersions[0])) {
          throw std::runtime_error("Validation failed");
        }
      }

      for (size_t p = 0; p < NUM_PLATFORMS; p++) {
        auto &rClasses = d.classes[p];

        for (auto &c : rClasses) {
          const uint32 hash = d.version1 ? MTHashV1(c.name) : MTHashV2(c.name);
          bool found = false;

          for (auto &f : title.fixups) {
            if (f.platform == p) {
              [&] {
                const ResourceClass *fClasses = f.classes.operator->();

                for (size_t i = 0; i < f.numItems; i++) {
                  if (fClasses[i].hash == hash) {
                    if (std::string_view(fClasses[i].extension) !=
                        c.extension) {
                      throw std::runtime_error("Validation error");
                    }
                    found = true;
                    return;
                  }
                }
                return;
              }();
            }
          }

          if (found) {
            continue;
          }

          auto foundClass = LowerBound(hash, shdr->resourceClasses[p]);

          if (foundClass == shdr->resourceClasses[p].end() ||
              foundClass->hash != hash) {
            foundClass = LowerBound(hash, shdr->resourceClasses[0]);

            if (foundClass == shdr->resourceClasses[0].end()) {
              throw std::runtime_error("Validation error");
            }
          }

          if (std::string_view(foundClass->extension) != c.extension) {
            throw std::runtime_error("Validation error");
          }

          [&] {
            for (auto cf :
                 ClassesFromExtension(*shdr, c.extension, d.version1)) {
              if (cf == hash) {
                return;
              }
            }

            throw std::runtime_error("Validation error");
          }();
        }
      }
    }

    for (auto &c : d.otherClasses) {
      const uint32 hash = d.version1 ? MTHashV1(c) : MTHashV2(c);
      auto found = LowerBound(hash, shdr->classes);

      if (found == shdr->classes.end() || found->hash != hash ||
          std::string_view(found->name) != c) {
        throw std::runtime_error("Validation error");
      }
    }
  }
}

int main() {
  LoadDb();

  std::stringstream str;
  BinWritterRef wr(str);
  wr.Write(Header{});

  std::map<std::string_view, uint32> sliderOffsets = DoStrings(wr);
  Header hdr = DoClasses(wr, sliderOffsets);
  const size_t indicesBegin = DoSupport(wr, sliderOffsets);

  wr.Push();
  wr.Seek(0);
  wr.Write(hdr);
  wr.Pop();

  auto titleOffsets = DoTitleFixups(
      wr, indicesBegin,
      reinterpret_cast<const Header *>(str.rdbuf()->view().data()),
      sliderOffsets);

  std::map<std::string_view, size_t, CompareString> titles;

  size_t curTitle = 0;
  for (auto &d : GLOBAL_DB) {
    for (auto &t : d.titles) {
      titles.emplace(t, titleOffsets.at(curTitle));
    }
    curTitle++;
  }

  hdr.titles.numItems = titles.size();
  hdr.titles.data.varPtr = wr.Tell() - offsetof(Header, titles);

  for (auto &[tit, off] : titles) {
    TitleName res{
        .name{{
            .size = uint32(tit.size()),
            .offset = int32(sliderOffsets.at(tit) - wr.Tell() -
                            offsetof(TitleName, name)),
        }},
        .data{.varPtr = int32(off - wr.Tell() - offsetof(TitleName, data))},
    };

    wr.Write(res);
  }

  wr.Seek(0);
  wr.Write(hdr);

  // BinWritter wrn("database/redb");
  // wrn.WriteContainer(str.rdbuf()->view());

  const uint64 *oData =
      reinterpret_cast<const uint64 *>(str.rdbuf()->view().data());
  const size_t numItems = (str.rdbuf()->view().size() + 7) / 8;
  std::ofstream ostr("database/redb.c");

  try {
    ValidateDb(reinterpret_cast<const Header *>(str.rdbuf()->view().data()));
  } catch (const std::exception &e) {
    ostr << "#pragma error \"" << e.what() << "\"" << std::endl;
    throw;
  }

  // C23 uses embed macro, wait for compiler support
  ostr << "#include <stdint.h>\nconst uint64_t REDB[] "
          "__attribute__((section(\".redb\"))) = {"
       << std::hex;

  for (size_t i = 0; i < numItems; i++) {
    ostr << "0x" << oData[i] << ',';
  }

  ostr << "};\n";

  return 0;
}
