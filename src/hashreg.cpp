#include "revil/hashreg.hpp"
#include "database.hpp"
#include "spike/except.hpp"
#include "spike/reflect/detail/reflector_class.hpp"
#include "spike/reflect/detail/reflector_enum.hpp"

extern "C" const revil::Header REDB;

using namespace revil;

REFLECT(ENUMERATION(Platform), ENUM_MEMBER(Auto), ENUM_MEMBER(Win32),
        ENUM_MEMBER(N3DS), ENUM_MEMBER(PS3), ENUM_MEMBER(X360),
        ENUM_MEMBER(CAFE), ENUM_MEMBER(NSW), ENUM_MEMBER(PS4),
        ENUM_MEMBER(Android), ENUM_MEMBER(IOS), ENUM_MEMBER(Win64));

namespace revil {

std::string_view GetExtension(uint32 hash, std::string_view title,
                              Platform platform_) {
  const uint8 platform = uint8(platform_) & PLATFORM_MASK;

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
std::vector<uint32> GetHash(std::string_view extension, std::string_view title,
                            Platform platform_) {
  auto supp = GetTitleSupport(title, platform_);
  const uint8 platform = uint8(platform_) & PLATFORM_MASK;

  std::vector<uint32> hashes;

  if (!title.empty()) {
    auto oKey = LowerBound(title, REDB.titles);

    if (oKey != REDB.titles.end() && std::string_view(oKey->name) == title) {
      for (auto &fx : oKey->data->fixups) {
        if (fx.platform == platform || fx.platform == 0) {
          const ResourceClass *fClasses = fx.classes.operator->();

          for (size_t i = 0; i < fx.numItems; i++) {
            if (std::string_view fExt(fClasses[i].extension); 
            fExt == extension) {
              hashes.emplace_back(fClasses[i].hash);
            }
          }
        }
      }
    }
  }

  if (hashes.empty()) {
    auto spn =
        ClassesFromExtension(REDB, extension, supp->arc.flags & DbArc_Version1);
    if (spn.size() > 0) {
      return {spn.begin(), spn.end()};
    }
  } else {
    return hashes;
  }

  // for backward compatibility, some extensions might have numerical (hashed)
  // extension (not found in main registry) if the extension has been added
  // later, just find it by hash and verify it in inverted registry
  auto cvted = strtoul(extension.data(), nullptr, 16);

  if (cvted < 0x10000) {
    return {};
  }

  auto extTranslated = GetExtension(cvted, title, platform_);

  if (extTranslated.empty()) {
    return {};
  }

  return GetHash(extTranslated, title, platform_);
}

Platforms GetPlatformSupport(std::string_view title) {
  auto found = LowerBound(title, REDB.titles);

  if (found == REDB.titles.end() || found->name != title) {
    throw es::RuntimeError("Coundn't find title.");
  }

  static const auto refl = GetReflectedEnum<Platform>();

  Platforms flags;

  for (uint32 i = 1; i < NUM_PLATFORMS; i++) {
    if (auto plat = found->data->support.operator->()[i - 1].operator->();
        plat) {
      flags.emplace_back(Platform(refl->values[i]));
    }
  }

  return flags;
}

const TitleSupport *GetTitleSupport(std::string_view title, Platform platform) {
  auto found = LowerBound(title, REDB.titles);

  if (found == REDB.titles.end() || found->name != title) {
    throw es::RuntimeError("Coundn't find title.");
  }

  if (platform == Platform::Auto) {
    platform = Platform::Win32;
  }

  auto platforms = found->data->support.operator->();

  auto foundSec = platforms[(uint8(platform) & PLATFORM_MASK) - 1].operator->();

  if (!foundSec && platform != Platform::Win32) {
    foundSec =
        platforms[(uint8(Platform::Win32) & PLATFORM_MASK) - 1].operator->();
  }

  if (!foundSec) {
    static const auto refl = GetReflectedEnum<Platform>();
    const bool inputPlatformBE = IsPlatformBigEndian(platform);

    for (uint32 i = 1; i < refl->numMembers; i++) {
      if (IsPlatformBigEndian(Platform(refl->values[i])) == inputPlatformBE &&
          platforms[i - 1].varPtr) {
        if (foundSec) {
          throw es::RuntimeError(
              "Ambiguous title support found from fallback.");
        }

        foundSec = platforms[i - 1].operator->();
      }
    }
  }

  if (!foundSec) {
    throw es::RuntimeError("Title support is null.");
  }

  return foundSec;
}

} // namespace revil
