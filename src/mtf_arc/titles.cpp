/*  Revil Format Library
    Copyright(C) 2020-2021 Lukas Cone

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

#include "datas/master_printer.hpp"
#include "ext_dd.hpp"
#include "ext_ddon.hpp"
#include "ext_dgs.hpp"
#include "ext_dgs2.hpp"
#include "ext_dmc4.hpp"
#include "ext_dr.hpp"
#include "ext_ext.hpp"
#include "ext_lp.hpp"
#include "ext_lp2.hpp"
#include "ext_mh3.hpp"
#include "ext_mh4.hpp"
#include "ext_mhg.hpp"
#include "ext_mhs.hpp"
#include "ext_mhs2.hpp"
#include "ext_pwaadd.hpp"
#include "ext_pwaasoj.hpp"
#include "ext_re0.hpp"
#include "ext_re5.hpp"
#include "ext_re6.hpp"
#include "ext_recv.hpp"
#include "ext_rem.hpp"
#include "ext_rer.hpp"
#include "ext_sb3.hpp"
#include "ext_sb4.hpp"
#include "ext_sb4s.hpp"
#include "ext_sbsh.hpp"
#include "ext_sbsyd.hpp"
#include "revil/hashreg.hpp"

// #define HRG_DEBUG

const std::map<std::string_view, const MtExtensions *> invertedExtensions{
    {"ace_attorney_dual_destinies", &extPWAADD},
    {"ace_attorney_spirit_of_justice", &extPWAASOJ},
    {"biohazzard_0", &extRE0},
    {"biohazzard_5", &extRE5},
    {"biohazzard_6", &extRE6},
    {"dai_gyakuten_saiban_2", &extDGS2},
    {"dai_gyakuten_saiban", &extDGS},
    {"dd", &extDD},
    {"ddon", &extDDON},
    {"dead_rising", &extDR},
    {"devil_may_cry_4", &extDMC4},
    {"dgs", &extDGS},
    {"dgs2", &extDGS2},
    {"dmc4", &extDMC4},
    {"dr", &extDR},
    {"dragons_dogma_online", &extDDON},
    {"dragons_dogma", &extDD},
    {"ex_troopers", &extEXT},
    {"ext", &extEXT},
    {"lost_planet_2", &extLP2},
    {"lost_planet", &extLP},
    {"lp", &extLP},
    {"lp2", &extLP2},
    {"mh3", &extMH3},
    {"mh4", &extMH4},
    {"mhg", &extMHG},
    {"mhs", &extMHS},
    {"mhs2", &extMHS2},
    {"mhx", &extMHG},
    {"mhxx", &extMHG},
    {"monster_hunter_3", &extMH3},
    {"monster_hunter_4", &extMH4},
    {"monster_hunter_cross", &extMHG},
    {"monster_hunter_double_cross", &extMHG},
    {"monster_hunter_generations", &extMHG},
    {"monster_hunter_stories_2", &extMHS2},
    {"monster_hunter_stories", &extMHS},
    {"monster_hunter_x", &extMHG},
    {"monster_hunter_xx", &extMHG},
    {"pwaadd", &extPWAADD},
    {"pwaasoj", &extPWAASOJ},
    {"re0", &extRE0},
    {"re5", &extRE5},
    {"re6", &extRE6},
    {"recv", &extRECV},
    {"rem", &extREM},
    {"rer", &extRER},
    {"resident_evil_0", &extRE0},
    {"resident_evil_5", &extRE5},
    {"resident_evil_6", &extRE6},
    {"resident_evil_code_veronica", &extRECV},
    {"resident_evil_mercenaries", &extREM},
    {"resident_evil_revelations", &extRER},
    {"sb3", &extSB3},
    {"sb4", &extSB4},
    {"sb4s", &extSB4S},
    {"sbsh", &extSBSH},
    {"sbsyd", &extSBSYD},
    {"sengoku_basara_3", &extSB3},
    {"sengoku_basara_4_sumeragi", &extSB4S},
    {"sengoku_basara_4", &extSB4},
    {"sengoku_basara_samurai_heroes", &extSBSH},
    {"sengoku_basara_sanada_yukimura_den", &extSBSYD},
};

const MtExtFixupStorage *GetFixups(std::string_view title) {
  auto found = invertedExtensions.find(title);

  if (es::IsEnd(invertedExtensions, found)) {
    throw std::runtime_error("Coundn't find title.");
  }

  return found->second->fixups;
}

namespace revil {
void LinkLogging(PrintFunc func, bool useColor) {
  es::print::AddPrinterFunction(func, useColor);
}

void GetTitles(TitleCallback cb) {
  for (auto &p : invertedExtensions) {
    cb(p.first);
  }
}

uint32 GetHash(std::string_view extension, std::string_view title,
               Platform platform) {
  auto found = invertedExtensions.find(title);

  if (es::IsEnd(invertedExtensions, found)) {
    throw std::runtime_error("Coundn't find title.");
  }

  auto foundSec = found->second;

  if (uint32 retVal = foundSec->GetHash(extension, platform); retVal) {
    return retVal;
  }

  // for backward compatibility, some extensions might have numerical (hashed)
  // extension (not found in main registry) if the extension has been added
  // later, just find it by hash and verify it in inverted registry
  auto cvted = strtoul(extension.data(), nullptr, 16);

  if (cvted < 0x10000) {
    return 0;
  }

  auto extTranslated = GetExtension(cvted, title, platform);

  if (extTranslated.empty()) {
    return 0;
  }

  return foundSec->GetHash(extTranslated, platform);
}

PlatformFlags GetPlatformSupport(std::string_view title) {
  auto found = invertedExtensions.find(title);

  if (es::IsEnd(invertedExtensions, found)) {
    throw std::runtime_error("Coundn't find title.");
  }

  auto foundSec = found->second;
  PlatformFlags flags;

  for (size_t i = 1; i < foundSec->NUMSLOTS; i++) {
    flags.Set(Platform(i), foundSec->data[i]);
  }

  return flags;
}

const TitleSupport *GetTitleSupport(std::string_view title, Platform platform) {
  auto found = invertedExtensions.find(title);

  if (es::IsEnd(invertedExtensions, found)) {
    throw std::runtime_error("Coundn't find title.");
  }

  auto foundSec = found->second->support;

  if (!foundSec) {
    throw std::runtime_error("Title support is null.");
  }

  return foundSec->Get(platform);
}

} // namespace revil

#ifdef HRG_DEBUG
static std::string_view shortNames[]{
    "dd",  "ddon", "dgs",  "dgs2",  "dmc4",   "dr",      "ext",  "lp",  "lp2",
    "mh3", "mh4",  "mhg",  "mhs",   "pwaadd", "pwaasoj", "re0",  "re5", "re6",
    "rem", "rer",  "sbsh", "sbsyd", "sb3",    "sb4",     "sb4s",
};

void RE_EXTERN CheckCollisions() {
  for (auto n : shortNames) {
    auto &registry = invertedExtensions.at(n);
    for (size_t p = 1; p < registry->NUMSLOTS; p++) {
      auto ptStore = registry->data[p];
      if (!ptStore) {
        continue;
      }

      auto check = [&](auto reg) {
        for (auto i : *reg) {
          auto name = GetExtension(i.second, {}, Platform(p));
          if (name != i.first) {
            if (registry->fixups) {
              auto found = registry->fixups->find(i.second);

              if (!es::IsEnd(*registry->fixups, found) &&
                  found->second == i.first) {
                continue;
              }

              name = GetExtension(i.second);

              if (name == i.first) {
                continue;
              }
            }

            printerror("Found collision for " << n << " platform: " << p << ": "
                                              << i.first << " vs " << name);
          }
        }
      };

      if (ptStore != registry->Base()) {
        check(registry->Base());
      }

      check(ptStore);
    }
  }
}

#endif
