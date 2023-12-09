#include "revil/hashreg.hpp"
#include "database.hpp"
#include "spike/reflect/detail/reflector_class.hpp"
#include "spike/reflect/detail/reflector_enum.hpp"
#include <stdexcept>

extern "C" const revil::Header REDB;

using namespace revil;

REFLECT(ENUMERATION(Platform), ENUM_MEMBER(Auto), ENUM_MEMBER(Win32),
        ENUM_MEMBER(PS3), ENUM_MEMBER(X360), ENUM_MEMBER(N3DS),
        ENUM_MEMBER(CAFE), ENUM_MEMBER(NSW), ENUM_MEMBER(PS4),
        ENUM_MEMBER(Android), ENUM_MEMBER(IOS), ENUM_MEMBER(Win64));

namespace revil {

std::string_view GetExtension(uint32 hash, std::string_view title,
                              Platform platform_) {
  const uint8 platform = uint8(platform_) & 077;

  if (!title.empty()) {
    auto oKey = LowerBound(title, REDB.titles);

    if (oKey != REDB.titles.end() && std::string_view(oKey->name) == title) {
      for (auto &fx : oKey->data->fixups) {
        if (fx.platform == platform) {
          const ResourceClass *fClasses = fx.classes.operator->();

          for (size_t i = 0; i < fx.numItems; i++) {
            if (fClasses[i].hash == hash) {
              return fClasses[i].extension;
            }
          }
        }
      }
    }
  }

  auto foundClass = LowerBound(hash, REDB.resourceClasses[platform]);

  if (foundClass != REDB.resourceClasses[platform].end() &&
      foundClass->hash == hash) {
    return foundClass->extension;
  }

  foundClass = LowerBound(hash, REDB.resourceClasses[0]);

  if (foundClass != REDB.resourceClasses[0].end() && foundClass->hash == hash) {
    return foundClass->extension;
  }

  return {};
}

std::string_view GetClassName(uint32 hash) {
  auto foundResClass = LowerBound(hash, REDB.resourceClasses[0]);

  if (foundResClass != REDB.resourceClasses[0].end() &&
      foundResClass->hash == hash) {
    return foundResClass->name;
  }

  auto foundClass = LowerBound(hash, REDB.classes);

  if (foundClass != REDB.classes.end() && foundClass->hash == hash) {
    return foundClass->name;
  }

  return {};
}

void GetTitles(TitleCallback cb) {
  for (auto &p : REDB.titles) {
    cb(p.name);
  }
}

/* Lookup order:
  extCommon map
  if platform not auto lookup class from ext<platform> map
*/
std::span<const uint32> GetHash(std::string_view extension,
                                std::string_view title, Platform platform) {
  auto supp = GetTitleSupport(title, platform);

  auto hashes =
      ClassesFromExtension(REDB, extension, supp->arc.flags & DbArc_Version1);

  if (hashes.size() > 0) {
    return hashes;
  }

  // for backward compatibility, some extensions might have numerical (hashed)
  // extension (not found in main registry) if the extension has been added
  // later, just find it by hash and verify it in inverted registry
  auto cvted = strtoul(extension.data(), nullptr, 16);

  if (cvted < 0x10000) {
    return {};
  }

  auto extTranslated = GetExtension(cvted, title, platform);

  if (extTranslated.empty()) {
    return {};
  }

  return GetHash(extTranslated, title, platform);
}

Platforms GetPlatformSupport(std::string_view title) {
  auto found = LowerBound(title, REDB.titles);

  if (found == REDB.titles.end() || found->name != title) {
    throw std::runtime_error("Coundn't find title.");
  }

  static const auto refl = GetReflectedEnum<Platform>();

  Platforms flags;

  for (uint32 i = 1; i < NUM_PLATFORMS; i++) {
    if (auto plat = found->data->support.operator->()[i].operator->(); plat) {
      flags.emplace_back(Platform(refl->values[i]));
    }
  }

  return flags;
}

const TitleSupport *GetTitleSupport(std::string_view title, Platform platform) {
  auto found = LowerBound(title, REDB.titles);

  if (found == REDB.titles.end() || found->name != title) {
    throw std::runtime_error("Coundn't find title.");
  }

  if (platform == Platform::Auto) {
    platform = Platform::Win32;
  }

  auto foundSec =
      found->data->support.operator->()[(uint8(platform) & 077) - 1].
      operator->();

  if (!foundSec && platform != Platform::Win32) {
    foundSec =
        found->data->support.operator->()[(uint8(Platform::Win32) & 077) - 1]
            .
            operator->();
  }

  if (!foundSec) {
    throw std::runtime_error("Title support is null.");
  }

  return foundSec;
}

} // namespace revil
