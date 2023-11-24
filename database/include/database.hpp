#pragma once
#include "revil/platform.hpp"
#include <cstring>
#include <span>

namespace revil {

template <class C> struct Pointer {
  int32 varPtr;

  const C *operator->() const {
    return reinterpret_cast<const C *>(
        (varPtr != 0) * reinterpret_cast<intptr_t>(&varPtr) + varPtr);
  }

  operator const C &() const { return *operator->(); }
};

template <class C> struct Array {
  Pointer<C> data;
  uint32 numItems;

  const C *begin() const { return data.operator->(); }
  const C *end() const { return begin() + numItems; }
};

struct Class {
  uint32 hash;
  StringEntry name;

  bool operator<(uint32 hsh) const { return hash < hsh; }
};

struct ResourceClass : Class {
  StringEntry extension;
};

struct Fixup {
  Pointer<ResourceClass> classes;
  uint8 platform;
  uint16 numItems;
};

struct Extension {
  StringEntry extension;
  uint32 hash;
};

struct Extension4 {
  StringEntry extension;
  uint32 hashes[4];
};

struct ExtensionN {
  StringEntry extension;
  SmallArray<uint32> hashes;
};

auto operator<=>(const revil::TitleSupport &t, const revil::TitleSupport &o) {
  int res = memcmp(&t, &o, sizeof(t));

  static const std::strong_ordering orders[]{
      std::strong_ordering::equal,
      std::strong_ordering::greater,
      std::strong_ordering::less,
  };

  static const int sign[]{1, 2};
  const int idx = (res != 0) * sign[res < 0];

  return orders[idx];
}

struct Title {
  Pointer<Pointer<revil::TitleSupport>> support;
  SmallArray<Fixup> fixups;
};

struct TitleName {
  StringEntry name;
  Pointer<Title> data;
};

constexpr uint32 NUM_PLATFORMS = (uint32(revil::Platform::Win64) & 077) + 1;

struct Header {
  Array<TitleName> titles;
  Array<ResourceClass> resourceClasses[NUM_PLATFORMS];
  Array<Class> classes;
  Array<Extension> extensions[2];
  Array<Extension4> extensions4[2];
  Array<ExtensionN> extensionsN;
};

inline bool operator<(const StringEntry &e, std::string_view sv) {
  if (e.size == sv.size()) {
    return std::string_view(e) < sv;
  }

  return e.size > sv.size();
}

inline bool operator<(const TitleName &e, std::string_view sv) {
  return e.name < sv;
}

inline bool operator<(const Extension &e, std::string_view sv) {
  return e.extension < sv;
}

inline bool operator<(const Extension4 &e, std::string_view sv) {
  return e.extension < sv;
}

inline bool operator<(const ExtensionN &e, std::string_view sv) {
  return e.extension < sv;
}

template <class Key, class OutKey, template <class A> class ArrayType>
const OutKey *LowerBound(const Key &key, const ArrayType<OutKey> &items) {
  const OutKey *base = items.begin();
  size_t len = items.numItems;

  while (len > 1) {
    const size_t half = len / 2;
    base += (base[half - 1] < key) * half;
    len -= half;
  }
  return base;
}

std::span<const uint32> ClassesFromExtension(const Header &hdr,
                                             std::string_view ext,
                                             bool version1 = false) {
  if (auto foundExt = LowerBound(ext, hdr.extensions[version1]);
      foundExt != hdr.extensions[version1].end() &&
      foundExt->extension == ext) {
    return {&foundExt->hash, 1};
  }

  if (auto foundExt = LowerBound(ext, hdr.extensions4[version1]);
      foundExt != hdr.extensions4[version1].end() &&
      foundExt->extension == ext) {
    return {foundExt->hashes, 4};
  }

  if (!version1) {
    if (auto foundExt = LowerBound(ext, hdr.extensionsN);
        foundExt != hdr.extensionsN.end() && foundExt->extension == ext) {
      return {foundExt->hashes.operator->(), foundExt->hashes.size};
    }
  }

  return {};
}
}
