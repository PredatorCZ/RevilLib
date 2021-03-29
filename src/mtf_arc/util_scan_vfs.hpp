/*  Revil Format Library Utility Functions: Scan VFS
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

#pragma once
#include "arc.hpp"
#include "datas/binreader.hpp"
#include "datas/directory_scanner.hpp"
#include "datas/master_printer.hpp"
#include "revil/hashreg.hpp"

template <class registry, class reader>
void ScanVFS(const std::string &path, const registry &reg, Platform platform, reader readFc) {
  DirectoryScanner scan;
  scan.AddFilter(".arc");
  scan.Scan(path);

  std::map<uint32, const std::string *> newHashes;
  std::map<uint32, const std::string *> missingHashes;
  std::map<uint32, es::string_view> usedHashes;

  for (auto &f : scan) {
    es::string_view sw(f);
    BinReader rd(f);
    try {
      ARC hdr;
      ARCFiles items;
      std::tie(hdr, items) = readFc(rd);

      for (auto &i : items) {
        auto found = GetExtension(i.typeHash, {}, platform);

        if (found.empty()) {
          if (!newHashes.count(i.typeHash)) {
            newHashes[i.typeHash] = &f;
          }
        } else {
          auto retHash = reg.GetHash(found, platform);

          if (!retHash) {
            if (!missingHashes.count(i.typeHash)) {
              missingHashes[i.typeHash] = &f;
            }
          } else {
            usedHashes[i.typeHash];
          }
        }
      }
    } catch (const std::exception &e) {
      printerror("Cannot process: " << f << " what: " << e.what());
    } catch (...) {
      printerror("Cannot process: " << f);
    }
  }

  if (!newHashes.empty()) {
    printline("New hashes:");

    for (auto &h : newHashes) {
      printline("0x" << std::hex << std::uppercase << h.first << " "
                     << *h.second);
    }
  }

  if (!missingHashes.empty()) {
    printline("Missing hashes:");

    for (auto &h : missingHashes) {
      printline(GetExtension(h.first, {}, platform)
                << " 0x" << std::hex << std::uppercase << h.first << " "
                << *h.second);
    }
  }

  if (!usedHashes.empty()) {
    printline("Unused hashes:");
    for (auto &h : *reg.Base()) {
      if (!usedHashes.count(h.second)) {
        printline(h.first);
      }
    }
  }
};
