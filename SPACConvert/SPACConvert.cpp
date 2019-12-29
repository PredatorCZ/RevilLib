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

#include "datas/MultiThread.hpp"
#include "datas/SettingsManager.hpp"
#include "datas/binreader.hpp"
#include "datas/binwritter.hpp"
#include "datas/endian.hpp"
#include "datas/fileinfo.hpp"
#include "datas/reflectorRegistry.hpp"
#include "formats/FWSE.hpp"
#include "formats/MSF.hpp"
#include "formats/WAVE.hpp"
#include "project.h"
#include "pugixml.hpp"

extern "C" {
#include "vgmstream.h"
}

static struct SPACConvert : SettingsManager {
  DECLARE_REFLECTOR;

  bool Generate_Log = false;
  bool Force_WAV_Conversion = false;
  bool Convert_to_WAV = true;
} settings;

REFLECTOR_START_WNAMES(SPACConvert, Convert_to_WAV, Force_WAV_Conversion,
                       Generate_Log);

static const char help[] = "\nConverts SPAC sound archives.\n\
Settings (.config file):\n\
  Convert_to_WAV: \n\
        Convert non WAV formats into a 16bit PCM WAV.\n\
  Force_WAV_Conversion:\n\
        Force conversion of all sounds to a 16bit PCM WAV.\n\
  Generate_Log: \n\
        Will generate text log of console output next to application location.\n\t";

static const char pressKeyCont[] = "\nPress ENTER to close.";

class VGMMemoryFile;

static thread_local VGMMemoryFile *currentFile = nullptr;

class VGMMemoryFile {
  STREAMFILE sf; // must be always first, to fool free(), reinterpret_casts, etc
  off_t bufferOffset = 0;

public:
  char *buffer;
  uint bufferSize;
  const char *fileName;

private:
  static size_t Read(STREAMFILE *fl, uint8_t *dest, off_t offset,
                     size_t length) {
    VGMMemoryFile *self = reinterpret_cast<VGMMemoryFile *>(fl);
    if (offset + length > self->bufferSize || !dest || !length)
      return 0;

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
  static constexpr int ID = 0x43415053;
  static constexpr int ID_R = 0x53504143;

  int id, version, numFiles, unkCount00, unkCount01, unkOffset00, unkOffset01,
      dataOffset;

  void SwapEndian() { FArraySwapper<int>(*this); }
};

REFLECTOR_CREATE(FileType, ENUM, 2, CLASS, EXTERN, WAV, FWSE, MSF);

struct ArchiveBuffer {
  struct FileEntry {
    char *start;
    int size;
    FileType fileType;

    FileEntry(char *iStart, int iLen, FileType iType)
        : start(iStart), size(iLen), fileType(iType) {}
  };

  char *buffer = nullptr;
  int curPos = 0;
  std::vector<FileEntry> entries;
  ~ArchiveBuffer() {
    if (buffer)
      free(buffer);
  }
};

void LoadSPAC(const TCHAR *fle) {
  printline("Loading file: ", << fle);

  TSTRING filepath = fle;
  BinReader rd(fle);

  if (!rd.IsValid()) {
    printerror("Cannot open file.");
    return;
  }

  SPACHeader hdr;
  rd.Read(hdr);

  if (hdr.id == SPACHeader::ID_R) {
    hdr.SwapEndian();
  } else if (hdr.id != SPACHeader::ID) {
    printerror("Invalid file format.");
    return;
  }

  size_t curBuffITer = hdr.dataOffset;
  ArchiveBuffer msBuffer;
  msBuffer.buffer = static_cast<char *>(malloc(rd.GetSize()));

  for (int f = 0; f < hdr.numFiles; f++) {
    int firstFileID;
    rd.SavePos();
    rd.Read(firstFileID);
    rd.RestorePos();

    if ((firstFileID & 0xffffff) == CompileFourCC("MSF\0")) {
      char *bufferStart = msBuffer.buffer + msBuffer.curPos;

      rd.ReadBuffer(bufferStart, sizeof(MSF));

      int msfDataSize = reinterpret_cast<MSF *>(bufferStart)->dataSize;
      FByteswapper(msfDataSize);

      rd.SavePos();
      rd.Seek(curBuffITer);
      rd.ReadBuffer(bufferStart + sizeof(MSF), msfDataSize);
      int fullSize = msfDataSize + sizeof(MSF);
      msBuffer.entries.emplace_back(bufferStart, fullSize, FileType::MSF);
      msBuffer.curPos += fullSize;
      rd.ApplyPadding(128);
      curBuffITer = rd.Tell();
      rd.RestorePos();
    } else if (firstFileID == RIFFHeader::ID) {
      char *bufferStart = msBuffer.buffer + msBuffer.curPos;
      char *bufferIter = bufferStart;

      rd.ReadBuffer(bufferStart, sizeof(RIFFHeader));
      bufferIter += sizeof(RIFFHeader);

      WAVEGenericHeader gHdr(0);

      while (true) {
        rd.SavePos();
        rd.Read(gHdr);
        rd.RestorePos();

        if (gHdr.id == RIFFHeader::ID)
          break;
        else if (gHdr.id == WAVE_data::ID) {
          rd.ReadBuffer(bufferIter, sizeof(WAVEGenericHeader));
          bufferIter += sizeof(WAVEGenericHeader);
          rd.SavePos();
          rd.Seek(curBuffITer);
          rd.ReadBuffer(bufferIter, gHdr.chunkSize);
          bufferIter += gHdr.chunkSize;
          curBuffITer = rd.Tell();
          rd.RestorePos();
        } else if (IsValidWaveChunk(gHdr)) {
          rd.ReadBuffer(bufferIter, sizeof(WAVEGenericHeader) + gHdr.chunkSize);

          if (gHdr.id == WAVE_smpl::ID) {
            WAVE_smpl *fmt = reinterpret_cast<WAVE_smpl *>(bufferIter);
            if (fmt->numSampleLoops) {
              WAVE_smpl::SampleLoop *cLoop = fmt->GetSampleLoops();

              if (!cLoop->end && !cLoop->start)
                fmt->numSampleLoops = 0;
            }
          }

          bufferIter += sizeof(WAVEGenericHeader) + gHdr.chunkSize;
        } else if (f + 1 < hdr.numFiles) {
          throw std::runtime_error("Invalid WAVE chunk!");
        } else {
          break;
        }
      }
      const int fullSize = bufferIter - bufferStart;
      msBuffer.entries.emplace_back(bufferStart, fullSize, FileType::WAV);
      msBuffer.curPos += fullSize;

    } else if (firstFileID == FWSE::ID) {
      char *bufferStart = msBuffer.buffer + msBuffer.curPos;

      rd.ReadBuffer(bufferStart, sizeof(FWSE));
      FWSE *fwseHdr = reinterpret_cast<FWSE *>(bufferStart);
      int fwseDataSize = fwseHdr->fileSize - fwseHdr->bufferOffset;

      if (fwseHdr->bufferOffset != 0x400)
        throw std::runtime_error("Invalid FWSE header length!");

      rd.SavePos();
      rd.Seek(curBuffITer);
      rd.ReadBuffer(bufferStart + sizeof(FWSE), fwseDataSize);
      msBuffer.entries.emplace_back(bufferStart, fwseHdr->fileSize,
                                    FileType::FWSE);
      msBuffer.curPos += fwseHdr->fileSize;
      curBuffITer = rd.Tell();
      rd.RestorePos();
    } else {
      throw std::runtime_error("Invalid entry format!");
    }
  }

  int currentFile = 0;
  VGMMemoryFile *nmFile =
      !settings.Convert_to_WAV ? nullptr : VGMMemoryFile::Create();

  for (auto &e : msBuffer.entries) {
    TFileInfo finf(fle);
    _tmkdir((finf.GetPath() + _T("out")).c_str());

    std::string nakedName =
        std::to_string(currentFile) + '.' +
        _EnumWrap<FileType>{}._reflected[static_cast<int>(e.fileType)];

    if (settings.Convert_to_WAV &&
        (settings.Force_WAV_Conversion || e.fileType != FileType::WAV)) {
      nmFile->buffer = e.start;
      nmFile->bufferSize = e.size;
      nmFile->fileName = nakedName.c_str();

      VGMSTREAM *cVGMStream = init_vgmstream_from_STREAMFILE(*nmFile);

      if (!cVGMStream) {
        printerror("VGMStream Error!");
        continue;
      }

      const size_t samplerSize = cVGMStream->num_samples * sizeof(sample_t);
      sample_t *sampleBuffer = static_cast<sample_t *>(malloc(samplerSize));

      render_vgmstream(sampleBuffer, cVGMStream->num_samples, cVGMStream);

      RIFFHeader hdr(sizeof(RIFFHeader) + sizeof(WAVE_fmt) + sizeof(WAVE_data) +
                     samplerSize);
      WAVE_fmt fmt(WAVE_FORMAT::PCM);
      fmt.sampleRate = cVGMStream->sample_rate;
      fmt.CalcData();
      WAVE_data wData(samplerSize);

      close_vgmstream(cVGMStream);

      TSTRING filename = finf.GetPath() + _T("out/") + finf.GetFileName() +
                         _T('_') + ToTSTRING(currentFile) + _T(".wav");

      BinWritter wr(filename);

      if (!wr.IsValid()) {
        printerror("Failed to create file: ", << filename);
        free(sampleBuffer);
        continue;
      }

      wr.Write(hdr);
      wr.Write(fmt);
      wr.Write(wData);

      wr.WriteBuffer(reinterpret_cast<char *>(sampleBuffer), samplerSize);

      free(sampleBuffer);
    } else {
      TSTRING filename = finf.GetPath() + _T("out/") + finf.GetFileName() +
                         _T('_') + esStringConvert<TCHAR>(nakedName.c_str());
      std::ofstream ofs(filename, std::ios::out | std::ios::binary);
      ofs.write(e.start, e.size);
      ofs.close();
    }

    currentFile++;
  }

  delete nmFile;

  return;
}

REGISTER_ENUMS(FileType);

struct SPACQueueTraits {
  int queue;
  int queueEnd;
  TCHAR **files;
  typedef void return_type;

  return_type RetreiveItem() { LoadSPAC(files[queue]); }

  operator bool() { return queue < queueEnd; }
  void operator++(int) { queue++; }
  int NumQueues() const { return queueEnd - 1; }
};

int _tmain(int argc, TCHAR *argv[]) {
  setlocale(LC_ALL, "");
  printer.AddPrinterFunction(wprintf);
  RegisterLocalEnums();

  printline(SPACConvert_DESC ", " SPACConvert_COPYRIGHT
                             "\nSimply drag'n'drop files into application or "
                             "use as " SPACConvert_PRODUCT_NAME
                             " file1 file2 ...\n");

  TFileInfo configInfo(*argv);
  const TSTRING configName =
      configInfo.GetPath() + configInfo.GetFileName() + _T(".config");

  settings.FromXML(configName);

  pugi::xml_document doc = {};
  pugi::xml_node mainNode(settings.ToXML(doc));
  mainNode.prepend_child(pugi::xml_node_type::node_comment).set_value(help);

  doc.save_file(configName.c_str(), "\t",
                pugi::format_write_bom | pugi::format_indent);

  if (argc < 2) {
    printerror("Insufficient argument count, expected at least 1.");
    printer << help << pressKeyCont >> 1;
    getchar();
    return 1;
  }

  if (argv[1][1] == '?' || argv[1][1] == 'h') {
    printer << help << pressKeyCont >> 1;
    getchar();
    return 0;
  }

  if (settings.Generate_Log)
    settings.CreateLog(configInfo.GetPath() + configInfo.GetFileName());

  printer.PrintThreadID(true);

  SPACQueueTraits spcQue;
  spcQue.files = argv;
  spcQue.queue = 1;
  spcQue.queueEnd = argc;

  RunThreadedQueue(spcQue);

  return 0;
}