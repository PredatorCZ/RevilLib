/*  TEXConvert
    Copyright(C) 2020 Lukas Cone

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
#include "datas/master_printer.hpp"
#include "datas/multi_thread.hpp"
#include "datas/pugiex.hpp"
#include "datas/reflector.hpp"
#include "datas/reflector_xml.hpp"
#include "datas/settings_manager.hpp"
#include "datas/stat.hpp"
#include "formats/DDS.hpp"
#include "project.h"
#include <algorithm>

static struct TEXConvert : SettingsManager<TEXConvert> {
  bool Generate_Log = false;
  bool Convert_DDS_to_legacy = true;
  bool Force_unconvetional_legacy_formats = true;
  bool Folder_scan_TEX_only = true;
  bool Extract_largest_mipmap = false;
} settings;

REFLECTOR_CREATE(
    TEXConvert, 1, EXTENDED,
    (D, Convert_DDS_to_legacy,
     "Tries to convert TEX into legacy (DX9) DDS format."),
    (D, Force_unconvetional_legacy_formats,
     "Will try to convert some matching formats from DX10 to DX9, for "
     "example: RG88 to AL88."),
    (D, Extract_largest_mipmap,
     "Will try to extract only highest mipmap.\nTexture musn't be converted "
     "back afterwards, unless you regenerate mipmaps!\nThis setting does not "
     "apply, if texture have arrays or is a cubemap!"),
    (D, Generate_Log,
     "Will generate text log of console output next to "
     "application location."));

static const char appHeader[] = RETEXConvert_DESC
    " v" RETEXConvert_VERSION ", " RETEXConvert_COPYRIGHT "Lukas Cone"
    "\nSimply drag'n'drop files/folders onto application or "
    "use as " RETEXConvert_NAME
    " path1 path2 ...\nTool can detect and scan folders.";

static const char configHelp[] = "For settings, edit .config file.";

struct RETEXMip {
  char *offset;
  uint32 pad;
  uint32 unk;
  uint32 size;
};

struct RETEX {
  static constexpr uint32 ID = CompileFourCC("TEX\0");
  uint32 id, version;
  uint16 width, height,
      depth; // vector field
  uint8 numMips;
  uint8 numArrays;
  DXGI_FORMAT format;
  int32 unk01; // -1
  uint32 unk;  // 4 = cubemap
  uint32 flags;

  const RETEXMip *Mips() const {
    return reinterpret_cast<const RETEXMip *>(this + 1);
  }

  void Fixup() {
    char *root = reinterpret_cast<char *>(this);
    RETEXMip *mips = reinterpret_cast<RETEXMip *>(this + 1);
    uint32 numTotalMips = numMips * numArrays;

    for (uint32 t = 0; t < numTotalMips; t++) {
      mips[t].offset = root + reinterpret_cast<uintptr_t>(mips[t].offset);
    }
  }
};

void FilehandleITFC(const std::string &fle) {
  printline("Loading file: " << fle);
  BinReader rd(fle);

  uint32 id;
  rd.Read(id);
  rd.Seek(0);

  if (id != RETEX::ID) {
    throw es::InvalidHeaderError(id);
  }

  const size_t fleSize = rd.GetSize();
  std::string buffer;
  rd.ReadContainer(buffer, fleSize);
  RETEX *tex = reinterpret_cast<RETEX *>(&buffer[0]);
  tex->Fixup();

  AFileInfo fleInfo0(fle);
  auto outFile = fleInfo0.GetFullPathNoExt().to_string() + ".dds";
  BinWritter wr(outFile);

  DDS ddtex = {};
  ddtex = DDSFormat_DX10;
  ddtex.dxgiFormat = tex->format;
  ddtex.width = tex->width;
  ddtex.height = tex->height;

  if (tex->depth > 1) {
    ddtex.depth = tex->depth;
    ddtex.flags += DDS::Flags_Depth;
    ddtex.caps01 += DDS_HeaderEnd::Caps01Flags_Volume;
  } else if (tex->unk != 4) {
    ddtex.arraySize = tex->numArrays;
  } else {
    ddtex.caps01 = decltype(ddtex.caps01)(
        DDS::Caps01Flags_CubeMap, DDS::Caps01Flags_CubeMap_NegativeX,
        DDS::Caps01Flags_CubeMap_NegativeY, DDS::Caps01Flags_CubeMap_NegativeZ,
        DDS::Caps01Flags_CubeMap_PositiveX, DDS::Caps01Flags_CubeMap_PositiveY,
        DDS::Caps01Flags_CubeMap_PositiveZ);
  }

  ddtex.NumMipmaps(settings.Extract_largest_mipmap ? 1 : tex->numMips);

  const uint32 sizetoWrite =
      !settings.Convert_DDS_to_legacy || ddtex.arraySize > 1 ||
              ddtex.ToLegacy(settings.Force_unconvetional_legacy_formats)
          ? ddtex.DDS_SIZE
          : ddtex.LEGACY_SIZE;

  if (settings.Convert_DDS_to_legacy && sizetoWrite == ddtex.DDS_SIZE) {
    printwarning("Couldn't convert DX10 dds to legacy.")
  }

  wr.WriteBuffer(reinterpret_cast<const char *>(&ddtex), sizetoWrite);

  const RETEXMip *mips = tex->Mips();
  const uint32 mipPerArray = tex->numMips;

  for (uint32 a = 0; a < tex->numArrays; a++) {
    for (uint32 m = 0; m < ddtex.mipMapCount; m++) {
      const RETEXMip &cMip = mips[m + mipPerArray * a];
      wr.WriteBuffer(cMip.offset, cMip.size * tex->depth);
    }
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
                              {ReflectorXMLUtil::Flags_ClassNode});
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
      sc.AddFilter(settings.Folder_scan_TEX_only ? ".tex." : ".dds");
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
    default:
      printerror("Invalid path: " << fileName);
      break;
    }
  }

  printer.PrintThreadID(true);

  RunThreadedQueue(files.size(), [&](size_t index) {
    try {
      FilehandleITFC(files[index]);
    } catch (const std::exception &e) {
      printerror(e.what());
    }
  });

  return 0;
}
