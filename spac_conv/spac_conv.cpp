/*      SPACConvert
        Copyright(C) 2019 Lukas Cone

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
#include "datas/endian.hpp"
#include "datas/fileinfo.hpp"
#include "datas/multi_thread.hpp"
#include "datas/pugiex.hpp"
#include "datas/reflector_xml.hpp"
#include "datas/settings_manager.hpp"
#include "datas/stat.hpp"
#include "formats/FWSE.hpp"
#include "formats/MSF.hpp"
#include "formats/WAVE.hpp"
#include "project.h"

extern "C" {
#include "vgmstream.h"
}

static struct SPACConvert : SettingsManager<SPACConvert> {
  bool Generate_Log = false;
  bool Force_WAV_Conversion = false;
  bool Convert_to_WAV = true;
} settings;

REFLECTOR_CREATE(SPACConvert, 1, VARNAMES, Convert_to_WAV, Force_WAV_Conversion,
                 Generate_Log);

static const char appHeader[] =
    SPACConvert_DESC " v" SPACConvert_VERSION ", " SPACConvert_COPYRIGHT "Lukas Cone"
                  "\nSimply drag'n'drop files/folders onto application or "
                  "use as " SPACConvert_NAME
                  " path1 path2 ...\nTool can detect and scan folders.";

static const char configHelp[] = "For settings, edit .config file.";

class VGMMemoryFile;

static thread_local VGMMemoryFile *currentFile = nullptr;

class VGMMemoryFile {
  STREAMFILE sf; // must be always first, to fool free(), reinterpret_casts, etc
  off_t bufferOffset = 0;

public:
  char *buffer;
  size_t bufferSize;
  const char *fileName;

private:
  static size_t Read(STREAMFILE *fl, uint8_t *dest, off_t offset,
                     size_t length) {
    VGMMemoryFile *self = reinterpret_cast<VGMMemoryFile *>(fl);
    if (offset + length > self->bufferSize || !dest || !length) {
      return 0;
    }

    char *cBufferPos = self->buffer + offset;
    self->bufferOffset = offset + length;
    memcpy(dest, cBufferPos, length);

    return length;
  }

  static size_t GetSize(STREAMFILE *fl) {
    VGMMemoryFile *self = reinterpret_cast<VGMMemoryFile *>(fl);

    return self->bufferSize;
  }

  static off_t GetOffset(STREAMFILE *fl) {
    VGMMemoryFile *self = reinterpret_cast<VGMMemoryFile *>(fl);

    return self->bufferOffset;
  }

  static void GetName(STREAMFILE *fl, char *buffer, size_t length) {
    VGMMemoryFile *self = reinterpret_cast<VGMMemoryFile *>(fl);

    strncpy(buffer, self->fileName, length);
    buffer[length - 1] = '\0';
  }

  static void Destroy(STREAMFILE *fl) {
    VGMMemoryFile *self = reinterpret_cast<VGMMemoryFile *>(fl);

    // currentFile = nullptr;
    // delete self;
  }

  static STREAMFILE *Create(STREAMFILE *, const char *fileName, size_t) {
    if (fileName)
      return *currentFile;

    VGMMemoryFile *memfile = new VGMMemoryFile();

    memfile->sf.read = Read;
    memfile->sf.get_size = GetSize;
    memfile->sf.get_offset = GetOffset;
    memfile->sf.get_name = GetName;
    memfile->sf.open = Create;
    memfile->sf.close = Destroy;

    currentFile = memfile;

    return *memfile;
  }

public:
  static VGMMemoryFile *Create() {
    return reinterpret_cast<VGMMemoryFile *>(Create(nullptr, nullptr, 0));
  }

  operator STREAMFILE *() { return reinterpret_cast<STREAMFILE *>(this); }
};

struct SPACHeader {
  static constexpr uint32 ID = CompileFourCC("SPAC");
  static constexpr uint32 ID_R = CompileFourCC("CAPS");

  uint32 id, version, numFiles, unkCount00, unkCount01, unkOffset00,
      unkOffset01, dataOffset;

  void SwapEndian() { FArraySwapper<int>(*this); }
};

REFLECTOR_CREATE(SPACFileType, ENUM, 1, CLASS, WAV, FWSE, MSF);

struct ArchiveFileEntry {
  char *start;
  size_t size;
  SPACFileType fileType;

  ArchiveFileEntry(char *iStart, size_t iLen, SPACFileType iType)
      : start(iStart), size(iLen), fileType(iType) {}
};

struct ArchiveBuffer {
  std::string buffer;
  size_t curPos = 0;
  std::vector<ArchiveFileEntry> entries;
};

void LoadSPAC(const std::string &fileName) {
  printline("Loading file: " << fileName);

  BinReader rd(fileName);

  SPACHeader hdr;
  rd.Read(hdr);

  if (hdr.id == SPACHeader::ID_R) {
    hdr.SwapEndian();
  } else if (hdr.id != SPACHeader::ID) {
    throw es::InvalidHeaderError(hdr.id);
  }

  size_t curBuffITer = hdr.dataOffset;
  ArchiveBuffer msBuffer;
  msBuffer.buffer.resize(rd.GetSize());

  for (size_t f = 0; f < hdr.numFiles; f++) {
    uint32 firstFileID;
    rd.Push();
    rd.Read(firstFileID);
    rd.Pop();

    if ((firstFileID & 0xffffff) == CompileFourCC("MSF\0")) {
      char *bufferStart = &msBuffer.buffer[0] + msBuffer.curPos;

      rd.ReadBuffer(bufferStart, sizeof(MSF));

      uint32 msfDataSize = reinterpret_cast<MSF *>(bufferStart)->dataSize;
      FByteswapper(msfDataSize);

      rd.Push();
      rd.Seek(curBuffITer);
      rd.ReadBuffer(bufferStart + sizeof(MSF), msfDataSize);
      size_t fullSize = msfDataSize + sizeof(MSF);
      msBuffer.entries.emplace_back(bufferStart, fullSize, SPACFileType::MSF);
      msBuffer.curPos += fullSize;
      rd.ApplyPadding(128);
      curBuffITer = rd.Tell();
      rd.Pop();
    } else if (firstFileID == RIFFHeader::ID) {
      char *bufferStart = &msBuffer.buffer[0] + msBuffer.curPos;
      char *bufferIter = bufferStart;

      rd.ReadBuffer(bufferStart, sizeof(RIFFHeader));
      bufferIter += sizeof(RIFFHeader);

      WAVEGenericHeader gHdr(0);

      while (true) {
        rd.Push();
        rd.Read(gHdr);
        rd.Pop();

        if (gHdr.id == RIFFHeader::ID) {
          break;
        } else if (gHdr.id == WAVE_data::ID) {
          rd.ReadBuffer(bufferIter, sizeof(WAVEGenericHeader));
          bufferIter += sizeof(WAVEGenericHeader);
          rd.Push();
          rd.Seek(curBuffITer);
          rd.ReadBuffer(bufferIter, gHdr.chunkSize);
          bufferIter += gHdr.chunkSize;
          curBuffITer = rd.Tell();
          rd.Pop();
        } else if (IsValidWaveChunk(gHdr)) {
          rd.ReadBuffer(bufferIter, sizeof(WAVEGenericHeader) + gHdr.chunkSize);

          if (gHdr.id == WAVE_smpl::ID) {
            WAVE_smpl *fmt = reinterpret_cast<WAVE_smpl *>(bufferIter);
            if (fmt->numSampleLoops) {
              WAVE_smpl::SampleLoop *cLoop = fmt->GetSampleLoops();

              if (!cLoop->end && !cLoop->start) {
                fmt->numSampleLoops = 0;
              }
            }
          }

          bufferIter += sizeof(WAVEGenericHeader) + gHdr.chunkSize;
        } else if (f + 1 < hdr.numFiles) {
          throw std::runtime_error("Invalid WAVE chunk!");
        } else {
          break;
        }
      }
      const size_t fullSize = bufferIter - bufferStart;
      msBuffer.entries.emplace_back(bufferStart, fullSize, SPACFileType::WAV);
      msBuffer.curPos += fullSize;

    } else if (firstFileID == FWSE::ID) {
      char *bufferStart = &msBuffer.buffer[0] + msBuffer.curPos;

      rd.ReadBuffer(bufferStart, sizeof(FWSE));
      FWSE *fwseHdr = reinterpret_cast<FWSE *>(bufferStart);
      size_t fwseDataSize = fwseHdr->fileSize - fwseHdr->bufferOffset;

      if (fwseHdr->bufferOffset != 0x400) {
        throw std::runtime_error("Invalid FWSE header length!");
      }

      rd.Push();
      rd.Seek(curBuffITer);
      rd.ReadBuffer(bufferStart + sizeof(FWSE), fwseDataSize);
      msBuffer.entries.emplace_back(bufferStart, fwseHdr->fileSize,
                                    SPACFileType::FWSE);
      msBuffer.curPos += fwseHdr->fileSize;
      curBuffITer = rd.Tell();
      rd.Pop();
    } else {
      throw std::runtime_error("Invalid entry format!");
    }
  }

  size_t currentFile = 0;
  VGMMemoryFile *nmFile =
      !settings.Convert_to_WAV ? nullptr : VGMMemoryFile::Create();

  for (auto &e : msBuffer.entries) {
    AFileInfo finf(fileName);
    auto outFolder = finf.GetFolder().to_string() + "out";
    es::mkdir(outFolder);
    auto extension =
        GetReflectedEnum<SPACFileType>().at(static_cast<size_t>(e.fileType));
    std::string nakedName =
        std::to_string(currentFile) + '.' + extension.to_string();

    if (settings.Convert_to_WAV &&
        (settings.Force_WAV_Conversion || e.fileType != SPACFileType::WAV)) {
      nmFile->buffer = e.start;
      nmFile->bufferSize = e.size;
      nmFile->fileName = nakedName.data();

      VGMSTREAM *cVGMStream = init_vgmstream_from_STREAMFILE(*nmFile);

      if (!cVGMStream) {
        printerror("VGMStream Error!");
        continue;
      }

      const size_t samplerSize = cVGMStream->num_samples * sizeof(sample_t);
      std::string samples;
      samples.resize(samplerSize);
      sample_t *sampleBuffer = reinterpret_cast<sample_t *>(&samples[0]);

      render_vgmstream(sampleBuffer, cVGMStream->num_samples, cVGMStream);

      RIFFHeader hdr(sizeof(RIFFHeader) + sizeof(WAVE_fmt) + sizeof(WAVE_data) +
                     samplerSize);
      WAVE_fmt fmt(WAVE_FORMAT::PCM);
      fmt.sampleRate = cVGMStream->sample_rate;
      fmt.CalcData();
      WAVE_data wData(samplerSize);

      close_vgmstream(cVGMStream);

      auto filename = finf.GetFolder().to_string() + "out/" +
                      finf.GetFilename().to_string() + '_' +
                      std::to_string(currentFile) + ".wav";

      try {
        BinWritter wr(filename);

        wr.Write(hdr);
        wr.Write(fmt);
        wr.Write(wData);

        wr.WriteBuffer(reinterpret_cast<char *>(sampleBuffer), samplerSize);
      } catch (const std::exception &e) {
        printerror(e.what());
      }

    } else {
      auto filename = finf.GetFolder().to_string() + "out/" +
                      finf.GetFilename().to_string() + '_' +
                      std::to_string(nakedName);
      std::ofstream ofs(filename, std::ios::out | std::ios::binary);
      ofs.write(e.start, e.size);
      ofs.close();
    }

    currentFile++;
  }

  delete nmFile;
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
      sc.AddFilter(".spc");
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
      LoadSPAC(files[index]);
    } catch (const std::exception &e) {
      printerror(e.what());
    }
  });

  return 0;
}
