/*  ValidateVFS
    Copyright(C) 2022 Lukas Cone

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

#include "../hfs.hpp"
#include "arc.hpp"
#include "datas/encrypt/blowfish.h"
#include "datas/fileinfo.hpp"
#include "datas/master_printer.hpp"
#include "ext_base.hpp"
#include "project.h"
#include "re_common.hpp"
#include "revil/hashreg.hpp"
#include <set>

static struct ValidateVFS : ReflectorBase<ValidateVFS> {
  std::string title;
  Platform platform = Platform::Auto;
} settings;

REFLECT(CLASS(ValidateVFS),
        MEMBER(title, "t", ReflDesc{"Set title for correct archive handling."}),
        MEMBER(platform, "p",
               ReflDesc{"Set platform for correct archive handling."}));

std::string_view filters[]{
    ".arc$",
};

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = ValidateVFS_DESC " v" ValidateVFS_VERSION
                               ", " ValidateVFS_COPYRIGHT "Lukas Cone",
    .settings = reinterpret_cast<ReflectorFriend *>(&settings),
    .filters = filters,
};

static const char DDONKey[] =
    "ABB(DF2I8[{Y-oS_CCMy(@<}qR}WYX11M)w[5V.~CbjwM5q<F1Iab+-";
static BlowfishEncoder enc;
static constexpr uint32 ARCCID = CompileFourCC("ARCC");

AppInfo_s *AppInitModule() {
  enc.SetKey(DDONKey);
  return &appInfo;
}

auto ReadARCC(BinReaderRef_e rd) {
  ARC hdr;
  rd.Read(hdr);
  rd.Skip(-4);

  if (hdr.id != ARCCID) {
    throw es::InvalidHeaderError(hdr.id);
  }

  ARCFiles files;
  rd.ReadContainer(files, hdr.numFiles);

  auto buffer = reinterpret_cast<char *>(files.data());
  size_t bufferSize = sizeof(ARCFile) * hdr.numFiles;

  enc.Decode(buffer, bufferSize);

  return std::make_tuple(hdr, files);
}

std::map<uint32, std::string> newHashes;
std::map<uint32, std::string> missingHashes;
std::set<uint32> usedHashes;
std::mutex mergeMtx;
const MtExtensions *reg = nullptr;

void AppProcessFile(AppContext *ctx) {
  std::stringstream backup;
  uint32 id;
  ctx->GetType(id);
  ARC hdr;
  BinReaderRef_e rd(ctx->GetStream());

  if (id == SFHID) {
    backup = ProcessHFS(rd);
    rd = BinReaderRef_e(backup);
    rd.Push();
    rd.Read(id);
    rd.Pop();
  }

  std::map<uint32, std::string> newHashes;
  std::map<uint32, std::string> missingHashes;
  std::set<uint32> usedHashes;

  if (!reg) {
    reg = revil::GetTitleRegistry(settings.title);
  }

  auto WriteFiles = [&](auto &files) {
    for (auto &f : files) {
      auto ext =
          revil::GetExtension(f.typeHash, settings.title, settings.platform);

      if (ext.empty()) {
        if (!newHashes.count(f.typeHash) && !::newHashes.count(f.typeHash)) {
          newHashes[f.typeHash] = f.fileName;
        }
      } else {
        auto retHash = reg->GetHash(ext, settings.platform);

        if (!retHash) {
          if (!missingHashes.count(f.typeHash) &&
              !::missingHashes.count(f.typeHash)) {
            missingHashes[f.typeHash] = f.fileName;
          }
        } else {
          usedHashes.emplace(f.typeHash);
        }
      }
    }
  };

  auto ts = revil::GetTitleSupport(settings.title, settings.platform);

  if (ts->arc.extendedFilePath) {
    ARCExtendedFiles files;
    std::tie(hdr, files) = ReadExtendedARC(rd);
    WriteFiles(files);
  } else {
    ARCFiles files;
    if (id == ARCCID) {
      std::tie(hdr, files) = ReadARCC(rd);
    } else {
      std::tie(hdr, files) = ReadARC(rd);
    }
    WriteFiles(files);
  }

  {
    std::lock_guard<std::mutex> lg(mergeMtx);

    for (auto &[k, v] : newHashes) {
      ::newHashes.emplace(k, std::move(v));
    }

    for (auto &[k, v] : missingHashes) {
      ::missingHashes.emplace(k, std::move(v));
    }

    for (auto k : usedHashes) {
      ::usedHashes.emplace(k);
    }
  }
}

void AppFinishContext() {
  if (!newHashes.empty()) {
    printline("New hashes:");

    for (auto &h : newHashes) {
      printline("0x" << std::hex << std::uppercase << h.first << " "
                     << h.second);
    }
  }

  if (!missingHashes.empty()) {
    printline("Missing hashes:");

    for (auto &h : missingHashes) {
      printline(GetExtension(h.first, {}, settings.platform)
                << " 0x" << std::hex << std::uppercase << h.first << " "
                << h.second);
    }
  }

  if (!usedHashes.empty()) {
    printline("Unused hashes:");
    for (auto &h : *reg->Base()) {
      if (!usedHashes.count(h.second)) {
        printline(h.first);
      }
    }
  }
}

size_t AppExtractStat(request_chunk requester) {
  auto data = requester(0, 32);
  HFS *hfs = reinterpret_cast<HFS *>(data.data());
  ARCBase *arcHdr = nullptr;

  if (hfs->id == SFHID) {
    arcHdr = reinterpret_cast<ARCBase *>(hfs + 1);
  } else {
    arcHdr = reinterpret_cast<ARCBase *>(hfs);
  }

  if (arcHdr->id == CRAID) {
    arcHdr->SwapEndian();
  } else if (arcHdr->id != ARCID) {
    return 0;
  }

  return arcHdr->numFiles;
}
