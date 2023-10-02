/*  SPACConvert
    Copyright(C) 2019-2022 Lukas Cone

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

#include "project.h"
#include "spike/app_context.hpp"
#include "spike/except.hpp"
#include "spike/format/FWSE.hpp"
#include "spike/format/MSF.hpp"
#include "spike/format/WAVE.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/io/binwritter_stream.hpp"
#include "spike/io/fileinfo.hpp"
#include "spike/master_printer.hpp"
#include "spike/reflect/reflector.hpp"
#include "spike/util/endian.hpp"
#include <memory>
#include <sstream>

#ifdef USE_VGM
#include "vgmstream.h"

static struct SPACConvert : ReflectorBase<SPACConvert> {
  bool forceWAV = false;
  bool convertWAV = true;
} settings;

REFLECT(
    CLASS(SPACConvert),
    MEMBERNAME(convertWAV, "convert-wav", "w",
               ReflDesc{"Convert sounds to WAV format."}),
    MEMBERNAME(
        forceWAV, "force-wav", "W",
        ReflDesc{
            "Convert ADPCM WAV files into PCM WAV if SPAC contains then."}));

class VGMMemoryFile;

static thread_local std::unique_ptr<VGMMemoryFile> currentFile;

class VGMMemoryFile {
  STREAMFILE sf; // must be always first, to fool free(), reinterpret_casts, etc
  offv_t bufferOffset = 0;

public:
  char *buffer;
  size_t bufferSize;
  const char *fileName;

private:
  static size_t Read(STREAMFILE *fl, uint8_t *dest, offv_t offset,
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

  static offv_t GetOffset(STREAMFILE *fl) {
    VGMMemoryFile *self = reinterpret_cast<VGMMemoryFile *>(fl);

    return self->bufferOffset;
  }

  static void GetName(STREAMFILE *fl, char *buffer, size_t length) {
    VGMMemoryFile *self = reinterpret_cast<VGMMemoryFile *>(fl);

    strncpy(buffer, self->fileName, length);
    buffer[length - 1] = '\0';
  }

  static void Destroy(STREAMFILE *fl) {
    // VGMMemoryFile *self = reinterpret_cast<VGMMemoryFile *>(fl);

    // currentFile = nullptr;
    // delete self;
  }

  static STREAMFILE *Create(STREAMFILE *, const char *fileName, size_t) {
    if (fileName)
      return reinterpret_cast<STREAMFILE *>(currentFile.get());

    currentFile = std::make_unique<VGMMemoryFile>();

    currentFile->sf.read = Read;
    currentFile->sf.get_size = GetSize;
    currentFile->sf.get_offset = GetOffset;
    currentFile->sf.get_name = GetName;
    currentFile->sf.open = Create;
    currentFile->sf.close = Destroy;

    return reinterpret_cast<STREAMFILE *>(currentFile.get());
  }

public:
  static VGMMemoryFile *Create() {
    return reinterpret_cast<VGMMemoryFile *>(Create(nullptr, nullptr, 0));
  }

  operator STREAMFILE *() { return reinterpret_cast<STREAMFILE *>(this); }
};

#endif

std::string_view filters[]{
    ".spc$",
};

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = SPACConvert_DESC " v" SPACConvert_VERSION
                               ", " SPACConvert_COPYRIGHT "Lukas Cone",
#ifdef USE_VGM
    .settings = reinterpret_cast<ReflectorFriend *>(&settings),
#endif
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

struct SPACHeader {
  static constexpr uint32 ID = CompileFourCC("SPAC");
  static constexpr uint32 ID_R = CompileFourCC("CAPS");

  uint32 id, version, numFiles, unkCount00, unkCount01, unkOffset00,
      unkOffset01, dataOffset;

  void SwapEndian() { FArraySwapper<int>(*this); }
};

MAKE_ENUM(ENUMSCOPE(class SPACFileType, SPACFileType), EMEMBER(WAV),
          EMEMBER(FWSE), EMEMBER(MSF), EMEMBER(XMA));

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

template <> void FByteswapper(WAVEGenericHeader &item, bool) {
  FByteswapper(item.id);
  FByteswapper(item.chunkSize);
}

template <> void FByteswapper(RIFFHeader &item, bool) {
  FByteswapper<WAVEGenericHeader>(item);
  FByteswapper(item.type);
}

template <> void FByteswapper(WAVE_data &item, bool) {
  FByteswapper<WAVEGenericHeader>(item);
}
/*
template <> void FByteswapper(WAVE_fmt &item, bool) {
  FByteswapper<WAVEGenericHeader>(item);
  FByteswapper(item.bitsPerSample);
  FByteswapper(item.channels);
  FByteswapper(item.chunkSize);
  FByteswapper(item.format);
  FByteswapper(item.interleave);
  FByteswapper(item.sampleCount);
  FByteswapper(item.sampleRate);
}

template <> void FByteswapper(WAVE_seek &item, bool) {
  FByteswapper<WAVEGenericHeader>(item);
  FByteswapper(item.data);
}

template <> void FByteswapper(WAVE_smpl &item, bool) {
  FByteswapper<WAVEGenericHeader>(item);
  FByteswapper(item.manufacturer);
  FByteswapper(item.MIDIPitchFraction);
  FByteswapper(item.MIDIUnityNote);
  FByteswapper(item.numSampleLoops);
  FByteswapper(item.product);
  FByteswapper(item.sampleLoopsSize);
  FByteswapper(item.samplePeriod);
  FByteswapper(item.SMPTEFormat);
  FByteswapper(item.SMPTEOffset);
}*/


void AppProcessFile(AppContext *ctx) {
  BinReaderRef_e rd(ctx->GetStream());

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
        BinReaderRef rdn(rd);
        rdn.Push();
        rdn.Read(gHdr);
        rdn.Pop();

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
    } else if (firstFileID == CompileFourCC("FFIR")) {
      char *bufferStart = &msBuffer.buffer[0] + msBuffer.curPos;
      char *bufferIter = bufferStart;

      RIFFHeader rhdr(0);
      rd.SwapEndian(true);
      rd.Read(rhdr);
      memcpy(bufferIter, &rhdr, sizeof(rhdr));
      bufferIter += sizeof(rhdr);

      WAVEGenericHeader gHdr(0);

      while (true) {
        rd.Push();
        rd.Read(gHdr);
        rd.Pop();

        if (gHdr.id == RIFFHeader::ID) {
          break;
        } else if (gHdr.id == WAVE_data::ID) {
          rd.ReadBuffer(bufferIter, sizeof(WAVEGenericHeader));
          WAVE_data *data = reinterpret_cast<WAVE_data *>(bufferIter);
          FByteswapper(*data);
          bufferIter += sizeof(WAVEGenericHeader);
          rd.Push();
          rd.Seek(curBuffITer);
          rd.ReadBuffer(bufferIter, gHdr.chunkSize);
          bufferIter += gHdr.chunkSize;
          curBuffITer = rd.Tell();
          rd.Pop();
        } else if (IsValidWaveChunk(gHdr)) {
          // VGMStream skips other chunks for now
          rd.ReadBuffer(bufferIter, sizeof(WAVEGenericHeader) + gHdr.chunkSize);
          WAVEGenericHeader *fmt = reinterpret_cast<WAVEGenericHeader *>(bufferIter);
          FByteswapper(*fmt);

          bufferIter += sizeof(WAVEGenericHeader) + gHdr.chunkSize;
        } else if (f + 1 < hdr.numFiles) {
          throw std::runtime_error("Invalid WAVE chunk!");
        } else {
          break;
        }
      }
      const size_t fullSize = bufferIter - bufferStart;
      msBuffer.entries.emplace_back(bufferStart, fullSize, SPACFileType::XMA);
      msBuffer.curPos += fullSize;
      rd.SwapEndian(false);
    }

    else {
      throw std::runtime_error("Invalid entry format!");
    }
  }

  size_t currentFile = 0;

#ifdef USE_VGM
  VGMMemoryFile *nmFile =
      !settings.convertWAV ? nullptr : VGMMemoryFile::Create();

  for (auto &e : msBuffer.entries) {
    AFileInfo &finf = ctx->workingFile;
    auto extension = GetReflectedEnum<SPACFileType>()
                         ->names[static_cast<size_t>(e.fileType)];
    std::string nakedName = (std::to_string(currentFile) + '.') + extension;
    auto ectx = ctx->ExtractContext();

    if (settings.convertWAV &&
        (settings.forceWAV || e.fileType != SPACFileType::WAV)) {
      nmFile->buffer = e.start;
      nmFile->bufferSize = e.size;
      nmFile->fileName = nakedName.data();

      VGMSTREAM *cVGMStream = init_vgmstream_from_STREAMFILE(*nmFile);

      if (!cVGMStream) {
        printerror("VGMStream Error!");
        continue;
      }

      vgmstream_info vgmInfo;
      describe_vgmstream_info(cVGMStream, &vgmInfo);

      const size_t samplerSize = vgmInfo.num_samples * sizeof(sample_t);
      std::string samples;
      samples.resize(samplerSize);
      sample_t *sampleBuffer = reinterpret_cast<sample_t *>(&samples[0]);

      render_vgmstream(sampleBuffer, vgmInfo.num_samples, cVGMStream);

      RIFFHeader hdr(sizeof(RIFFHeader) + sizeof(WAVE_fmt) + sizeof(WAVE_data) +
                     samplerSize);
      WAVE_fmt fmt(WAVE_FORMAT::PCM);
      fmt.sampleRate = vgmInfo.sample_rate;
      fmt.CalcData();
      WAVE_data wData(samplerSize);

      close_vgmstream(cVGMStream);

      auto filename = std::string(finf.GetFilename()) + '_' +
                      std::to_string(currentFile) + ".wav";

      ectx->NewFile(filename);
      std::stringstream str;
      BinWritterRef wr(str);
      wr.Write(hdr);
      wr.Write(fmt);
      wr.Write(wData);
      auto strBuff = std::move(str).str();
      ectx->SendData(strBuff);
      ectx->SendData({reinterpret_cast<char *>(sampleBuffer), samplerSize});
    } else {
      auto filename = std::string(finf.GetFilename()) + '_' + nakedName;
      ectx->NewFile(filename);
      ectx->SendData({e.start, e.size});
    }

    currentFile++;
  }

  delete nmFile;
#else
  for (auto &e : msBuffer.entries) {
    AFileInfo &finf = ctx->workingFile;
    auto extension = GetReflectedEnum<SPACFileType>()
                         ->names[static_cast<size_t>(e.fileType)];
    std::string nakedName = (std::to_string(currentFile) + '.') + extension;
    auto ectx = ctx->ExtractContext();
    auto filename = std::string(finf.GetFilename()) + '_' + nakedName;
    ectx->NewFile(filename);
    ectx->SendData({e.start, e.size});
    currentFile++;
  }
#endif
}
