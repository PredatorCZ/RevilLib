/*  DDONSngw
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

#include "datas/binreader.hpp"
#include "datas/binwritter.hpp"
#include "datas/directory_scanner.hpp"
#include "datas/multi_thread.hpp"
#include "datas/pugiex.hpp"
#include "datas/reflector_xml.hpp"
#include "datas/settings_manager.hpp"
#include "datas/stat.hpp"
#include "datas/vectors_simd.hpp"
#include "project.h"

static struct DDONSngw : SettingsManager<DDONSngw> {
  bool Encrypt = false;
  bool Generate_Log = false;
} settings;

REFLECTOR_CREATE(DDONSngw, 1, EXTENDED,
                 (D, Encrypt, "Switch between encrypt or decrypt only."),
                 (D, Generate_Log,
                  "Will generate text log of console output next to "
                  "application location."));

static const char appHeader[] =
    DDONSngw_DESC " v" DDONSngw_VERSION ", " DDONSngw_COPYRIGHT "Lukas Cone"
                  "\nSimply drag'n'drop files/folders onto application or "
                  "use as " DDONSngw_NAME
                  " path1 path2 ...\nTool can detect and scan folders.";

static const char configHelp[] = "For settings, edit .config file.";

constexpr uint32 SNGWID = CompileFourCC("OggS");
const UIVector4A16 KEY = 0x64958637;
const UIVector4A16 LMASK = 0x0f0f0f0f;
const UIVector4A16 RMASK = ~LMASK;

UIVector4A16 Decrypt(UIVector4A16 input) {
  const auto ldata = _mm_slli_epi64(_mm_and_si128(input._data, LMASK._data), 4);
  const auto rdata = _mm_srli_epi64(_mm_and_si128(input._data, RMASK._data), 4);

  return _mm_xor_si128(_mm_or_si128(ldata, rdata), KEY._data);
}

UIVector4A16 Encrypt(UIVector4A16 input) {
  input._data = _mm_xor_si128(input._data, KEY._data);
  const auto ldata = _mm_slli_epi64(_mm_and_si128(input._data, LMASK._data), 4);
  const auto rdata = _mm_srli_epi64(_mm_and_si128(input._data, RMASK._data), 4);

  return _mm_or_si128(ldata, rdata);
}

void ProcessFile(const std::string &fileName) {
  BinReader rd(fileName);
  uint32 id;
  rd.Read(id);
  rd.Seek(0);
  const size_t fileSize = rd.GetSize();
  const size_t numChunks = fileSize / sizeof(UIVector4A16);
  const size_t numRest = fileSize % sizeof(UIVector4A16);
  const size_t numItems = numChunks + (numRest ? 1 : 0);
  std::vector<UIVector4A16> store;

  if (id == SNGWID && settings.Encrypt) {
    printline("Encrypting " << fileName);

    rd.ReadContainer(store, numItems);
    memset(store.data(), 0, sizeof(SNGWID));

    for (size_t i = 0; i < numItems; i++) {
      store[i] = Encrypt(store[i]);
    }

    BinWritter wr(fileName + ".enc");
    wr.WriteBuffer(reinterpret_cast<const char *>(store.data()), fileSize);
  } else if (!settings.Encrypt) {
    printline("Decrypting " << fileName);
    UIVector4A16 sample;
    rd.Read(sample);
    rd.Seek(0);
    sample = Decrypt(sample);

    if (sample.X) {
      throw std::runtime_error("Decryption error!");
    }

    rd.ReadContainer(store, numItems);

    for (size_t i = 0; i < numItems; i++) {
      store[i] = Decrypt(store[i]);
    }

    memcpy(store.data(), &SNGWID, sizeof(SNGWID));

    BinWritter wr(fileName + ".dec");
    wr.WriteBuffer(reinterpret_cast<const char *>(store.data()), fileSize);
  }
}

int _tmain(int argc, TCHAR *argv[]) {
  setlocale(LC_ALL, "");
  printer.AddPrinterFunction(UPrintf);
  printline(appHeader);

  AFileInfo configInfo(std::to_string(*argv));
  auto configName = configInfo.GetFullPathNoExt().to_string() + ".config";
  try {
    auto doc = XMLFromFile(configName);
    ReflectorXMLUtil::LoadV2(settings, doc, true);
  } catch (const es::FileNotFoundError &e) {
  }
  {
    pugi::xml_document doc = {};
    std::stringstream str;
    settings.GetHelp(str);
    auto buff = str.str();
    doc.append_child(pugi::node_comment).set_value(buff.data());

    ReflectorXMLUtil::SaveV2a(settings, doc,
                              {ReflectorXMLUtil::Flags_ClassNode,
                               ReflectorXMLUtil::Flags_StringAsAttribute});
    XMLToFile(configName, doc,
              {XMLFormatFlag::WriteBOM, XMLFormatFlag::IndentAttributes});
  }

  if (argc < 2) {
    printerror("Insufficient argument count, expected at least 1.");
    printline(configHelp);
    return 1;
  }

  if (IsHelp(argv[1])) {
    printline(configHelp);
    return 0;
  }

  if (settings.Generate_Log) {
    settings.CreateLog(configInfo.GetFullPathNoExt().to_string());
  }

  std::vector<std::string> files;

  for (int a = 1; a < argc; a++) {
    auto fileName = std::to_string(argv[a]);
    auto type = FileType(fileName);

    switch (type) {
    case FileType_e::Directory: {
      DirectoryScanner sc;
      sc.AddFilter(".sngw");
      printline("Scanning: " << fileName);
      sc.Scan(fileName);
      printline("Files found: " << sc.Files().size());

      std::transform(std::make_move_iterator(sc.begin()),
                     std::make_move_iterator(sc.end()),
                     std::back_inserter(files),
                     [](auto &&item) { return std::move(item); });

      break;
    }
    case FileType_e::File:
      files.emplace_back(std::move(fileName));
      break;
    default: {
      printerror("Invalid path: " << fileName);
      break;
    }
    }
  }

  printer.PrintThreadID(true);

  RunThreadedQueue(files.size(), [&](size_t index) {
    try {
      ProcessFile(files[index]);
    } catch (const std::exception &e) {
      printerror(e.what());
    }
  });

  return 0;
}
