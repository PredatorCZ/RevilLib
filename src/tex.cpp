
#include "revil/tex.hpp"
#include "datas/binreader_stream.hpp"
#include "datas/binwritter_stream.hpp"
#include "datas/bitfield.hpp"
#include "datas/except.hpp"
#include "formats/addr_ps3.hpp"
#include <map>
#include <vector>

using namespace revil;

enum class TextureType : uint8 {
  None,
  ColorPixel,
  General,
  Cubemap,
  Volume,
};

enum class TextureTypeV2 {
  General = 2,
  Volume = 3,
  Cubemap = 6,
};

enum class GeneralTextureType {
  None,
  IllumMap,  // IM
  ColorMap,  // BM, LM, or SRGB??
  NormalMap, // NM XGXA
};

enum class CubemapTextureType {
  Eye, // LP PC, some struct dump
  Classic,
};

enum class TEXFormat : uint32 {
  DXT1 = CompileFourCC("DXT1"),
  DXT2 = CompileFourCC("DXT2"),
  DXT3 = CompileFourCC("DXT3"),
  DXT5 = CompileFourCC("DXT5"),
  RGBA8_PACKED = 0x15,
  RG8_SNORM = 0x3c,

  /*360
  405274959
  405275014
  438305106
  438305107
  438305108
  438305137
  438305147
  438305148*/
};

enum class TEXFormatV2 : uint8 {
  RGBA16F = 0x2,
  DXT1 = 0x13,
  DXT3 = 0x15,
  DXT5 = 0x17,
  DXT1_Gray = 0x19, // BC4 for some?
  DXT1_NM = 0x1e,
  DXT5_NM = 0x1f,
  DXT5_LM = 0x20, // rgb lightmap, alpha ambient occlusion?
  DXT5_PM = 0x25, // rgb NM, alpha HM
  DXT5_ID = 0x2a,
  RGBA8 = 0x27,
};

static constexpr uint32 TEXID = CompileFourCC("TEX");
static constexpr uint32 XETID = CompileFourCC("\0XET");

struct TEXx70 {
  using TextureType = BitMemberDecl<0, 4>;
  using TextureSubtype = BitMemberDecl<1, 4>;
  using TextureLayout = BitFieldType<uint16, TextureType, TextureSubtype>;
  uint32 id = TEXID;
  uint16 version;
  TextureLayout type;
  uint8 numMips;
  uint8 numFaces = 1; // 6 for cubemap
  uint16 null = 0;
  uint16 width;
  uint16 height;
  uint32 arraySize = 0;
  TEXFormat fourcc;
  Vector4 colorCorrection{1.f, 1.f, 1.f, 0};

  void SwapEndian(bool way) {
    FByteswapper(id);
    FByteswapper(version);
    FByteswapper(type, way);
    FByteswapper(width);
    FByteswapper(height);
    FByteswapper(arraySize);
    FByteswapper(fourcc);
    FByteswapper(colorCorrection);
  }
};

struct TEXx66 {
  using TextureType = BitMemberDecl<0, 4>;
  using TextureSubtype = BitMemberDecl<1, 4>;
  using TextureLayout = BitFieldType<uint16, TextureType, TextureSubtype>;
  uint32 id = TEXID;
  uint16 version;
  TextureLayout type;
  uint8 numMips;
  uint8 numFaces = 1;
  uint16 width;
  uint16 height;
  uint16 arraySize;
  TEXFormat fourcc;
  Vector4 colorCorrection{1.f, 1.f, 1.f, 0};

  void SwapEndian(bool way) {
    FByteswapper(id);
    FByteswapper(version);
    FByteswapper(type, way);
    FByteswapper(width);
    FByteswapper(height);
    FByteswapper(arraySize);
    FByteswapper(fourcc);
    FByteswapper(colorCorrection);
  }
};

struct TEXx56 {
  enum class TextureLayout : uint8 { General, Illum, Corrected = 4 };

  uint32 id;
  uint8 version;
  TextureType type;
  TextureLayout layout;
  uint8 numMips;
  uint32 width;
  uint32 height;
  uint32 arraySize;
  TEXFormat fourcc;

  void SwapEndian() {
    FByteswapper(id);
    FByteswapper(width);
    FByteswapper(height);
    FByteswapper(arraySize);
    FByteswapper(fourcc);
  }
};

struct TEXx87 {
  using TextureType = BitMemberDecl<0, 4>;
  using NumMips = BitMemberDecl<1, 5>;
  using NumFaces = BitMemberDecl<2, 8>;
  using Width = BitMemberDecl<3, 13>;
  using Tier0 = BitFieldType<uint32, TextureType, NumMips, NumFaces, Width>;
  using Height = BitMemberDecl<0, 13>;
  using Depth = BitMemberDecl<1, 13>;
  using Null = BitMemberDecl<2, 6>;
  using Tier1 = BitFieldType<uint32, Height, Depth, Null>;

  uint32 id;
  uint16 version;
  uint16 null;
  Tier0 tier0;
  Tier1 tier1;
  TEXFormatV2 format;

  void SwapEndian(bool way) {
    FByteswapper(id);
    FByteswapper(version);
    FByteswapper(tier0, way);
    FByteswapper(tier1, way);
  }
};

struct TEXCubemapData {
  float unk[27];
};

DDS_HeaderDX10 ConvertTEXFormat(TEXFormat fmt) {
  switch (fmt) {
  case TEXFormat::DXT1:
    return DXGI_FORMAT_BC1_UNORM;
  case TEXFormat::DXT2: {
    DDS_HeaderDX10 retVal(DXGI_FORMAT_BC2_UNORM);
    retVal.alphaMode = DDS_HeaderDX10::AlphaMode_Premultiplied;
    return retVal;
  }
  case TEXFormat::DXT3:
    return DXGI_FORMAT_BC2_UNORM;
  case TEXFormat::DXT5:
    return DXGI_FORMAT_BC3_UNORM;
  case TEXFormat::RGBA8_PACKED:
    return DXGI_FORMAT_R8G8B8A8_UNORM;
  case TEXFormat::RG8_SNORM:
    return DXGI_FORMAT_R8G8_SNORM;

  default:
    throw std::runtime_error("Unknown texture format!");
  }
}

DDS_HeaderDX10 ConvertTEXFormat(TEXFormatV2 fmt) {
  switch (fmt) {
  case TEXFormatV2::DXT1:
  case TEXFormatV2::DXT1_Gray:
  case TEXFormatV2::DXT1_NM:
    return DXGI_FORMAT_BC1_UNORM;
  case TEXFormatV2::DXT3:
    return DXGI_FORMAT_BC2_UNORM;
  case TEXFormatV2::DXT5:
  case TEXFormatV2::DXT5_NM:
  case TEXFormatV2::DXT5_LM:
  case TEXFormatV2::DXT5_PM:
  case TEXFormatV2::DXT5_ID:
    return DXGI_FORMAT_BC3_UNORM;
  case TEXFormatV2::RGBA16F:
    return DXGI_FORMAT_R16G16B16A16_FLOAT;
  case TEXFormatV2::RGBA8:
    return DXGI_FORMAT_R8G8B8A8_UNORM;

  default:
    throw std::runtime_error("Unknown texture format!");
  }
}

struct TEXInternal : TEX {
  void ConvertBuffer(Platform platform) {
    if (asDDS.dxgiFormat == DXGI_FORMAT_R8G8_SNORM) {
      const size_t stride = sizeof(__m128i);
      const size_t numLoops = buffer.size() / stride;
      const size_t restBytes = buffer.size() % stride;

      auto process = [](const char *data) {
        __m128i xmm = _mm_loadu_si128(reinterpret_cast<const __m128i *>(data));
        const __m128i xmn = _mm_set1_epi8(0x80);
        return _mm_add_epi8(xmm, xmn);
      };

      for (size_t i = 0; i < numLoops; i++) {
        const size_t index = i * stride;
        const __m128i value = process(buffer.data() + index);
        memcpy(&buffer[index], &value, stride);
      }

      if (restBytes) {
        const size_t index = numLoops * stride;
        const __m128i value = process(buffer.data() + index);
        memcpy(&buffer[index], &value, restBytes);
      }

      asDDS.dxgiFormat = DXGI_FORMAT_R8G8_UNORM;
    } else if (asDDS.dxgiFormat == DXGI_FORMAT_R8G8B8A8_UNORM &&
               platform == Platform::PS3 && IsPow2(asDDS.width) &&
               IsPow2(asDDS.height)) {
      std::string oldBuffer = buffer;
      MortonSettings mset(asDDS.width, asDDS.height);
      const size_t stride = sizeof(uint32);

      for (size_t p = 0; p < buffer.size(); p += stride) {
        const size_t coord = p >> 2;
        const size_t x = coord % asDDS.width;
        const size_t y = coord / asDDS.width;
        memcpy(&buffer[p], oldBuffer.data() + MortonAddr(x, y, mset) * stride,
               stride);
        FByteswapper(reinterpret_cast<uint32 &>(buffer[p]));
      }
    }
  }
};

TEX LoadTEXx56(BinReaderRef rd) {
  TEXInternal main;
  TEXx56 header;
  rd.Read(header);

  if (header.layout == TEXx56::TextureLayout::Corrected) {
    rd.Read(main.color);
  }

  if (header.type == TextureType::Volume) {
    rd.Read(static_cast<DDS_Header &>(main.asDDS));
    rd.Read(static_cast<DDS_PixelFormat &>(main.asDDS));
    rd.Read(static_cast<DDS_HeaderEnd &>(main.asDDS));
  } else if (header.type == TextureType::Cubemap) {
    throw std::runtime_error("Cubemaps are not supported.");
  } else {
    main.asDDS.width = header.width;
    main.asDDS.height = header.height;
    main.asDDS.depth = header.arraySize;
    main.asDDS.NumMipmaps(header.numMips);
    main.asDDS = DDSFormat_DX10;
    main.asDDS = ConvertTEXFormat(header.fourcc);
  }

  main.asDDS.ComputeBPP();
  size_t bufferSize = main.asDDS.ComputeBufferSize(main.mips);

  if (header.arraySize) {
    bufferSize *= header.arraySize;
  }

  rd.ReadContainer(main.buffer, bufferSize);
  main.ConvertBuffer(Platform::WinPC);

  return main;
}

template <class header_type>
TEX LoadTEXx66(BinReaderRef rd, Platform platform) {
  TEXInternal main;
  header_type header;
  rd.Read(header);

  main.asDDS.width = header.width;
  main.asDDS.height = header.height;
  main.asDDS.depth = header.arraySize;
  main.asDDS.NumMipmaps(header.numMips);
  main.asDDS = DDSFormat_DX10;
  main.asDDS = ConvertTEXFormat(header.fourcc);
  main.color = Vector4A16(header.colorCorrection);

  TextureType type = static_cast<TextureType>(
      header.type.template Get<typename header_type::TextureType>());

  if (type == TextureType::Volume) {
    main.asDDS.caps01 += DDS_HeaderEnd::Caps01Flags_Volume;
  } else if (type == TextureType::Cubemap) {
    throw std::runtime_error("Cubemaps are not supported.");
  }

  std::vector<uint32> offsets;
  rd.ReadContainer(offsets, header.numFaces * header.numMips);

  main.asDDS.ComputeBPP();
  size_t bufferSize = main.asDDS.ComputeBufferSize(main.mips);

  if (header.arraySize) {
    bufferSize *= header.arraySize;
  }

  rd.ReadContainer(main.buffer, bufferSize);
  main.ConvertBuffer(platform);

  return main;
}

TEX LoadTEXx87(BinReaderRef rd, Platform platform) {
  TEXInternal main;
  TEXx87 header;
  rd.Read(header);
  using t = TEXx87;

  main.asDDS.width = header.tier0.Get<t::Width>();
  main.asDDS.height = header.tier1.Get<t::Height>();
  main.asDDS.depth = header.tier1.Get<t::Depth>();
  main.asDDS.NumMipmaps(header.tier0.Get<t::NumMips>());
  main.asDDS = DDSFormat_DX10;
  main.asDDS = ConvertTEXFormat(header.format);

  TextureTypeV2 type = (TextureTypeV2)header.tier0.Get<t::TextureType>();

  if (type == TextureTypeV2::Volume) {
    main.asDDS.caps01 += DDS_HeaderEnd::Caps01Flags_Volume;
  } else if (type == TextureTypeV2::Cubemap) {
    throw std::runtime_error("Cubemaps are not supported.");
  }

  std::vector<uint32> offsets;
  rd.ReadContainer(offsets, main.asDDS.depth * main.asDDS.mipMapCount);

  main.asDDS.ComputeBPP();
  size_t bufferSize = main.asDDS.ComputeBufferSize(main.mips);

  if (main.asDDS.depth) {
    bufferSize *= main.asDDS.depth;
  }

  rd.ReadContainer(main.buffer, bufferSize);

  return main;
}

static const std::map<uint16, TEX (*)(BinReaderRef, Platform)> texLoaders{
    {0x66, LoadTEXx66<TEXx66>}, {0x70, LoadTEXx66<TEXx70>}, {0x87, LoadTEXx87}};

void TEX::Load(BinReaderRef rd, Platform platform) {
  struct {
    uint32 id;
    union {
      uint8 versionV10;
      uint16 versionV11;
      uint32 versionV20;
    };
  } header{};

  rd.Read(header);
  rd.Seek(0);

  if (header.id == XETID) {
    rd.SwapEndian(true);
  } else if (header.id != TEXID) {
    throw es::InvalidHeaderError(header.id);
  }

  if (header.versionV10 == 0x56) {
    if (rd.SwappedEndian()) {
      throw std::runtime_error("X360 texture format is unsupported.");
    }

    *this = LoadTEXx56(rd);
    return;
  } else if (rd.SwappedEndian()) {
    FByteswapper(header.versionV11);
  }

  if (platform == Platform::Auto) {
    platform = rd.SwappedEndian() ? Platform::PS3 : Platform::WinPC;
  }

  auto found = texLoaders.find(header.versionV11);

  if (!es::IsEnd(texLoaders, found)) {
    *this = found->second(rd, platform);
    return;
  }

  if (rd.SwappedEndian()) {
    FByteswapper(header.versionV20);
  }

  found = texLoaders.find(header.versionV10);

  if (es::IsEnd(texLoaders, found)) {
    throw es::InvalidVersionError();
  }

  *this = found->second(rd, platform);
  return;
}

void TEX::SaveAsDDS(BinWritterRef wr, Tex2DdsSettings settings) {
  size_t headerSize = asDDS.dxgiFormat ? DDS::DDS_SIZE : DDS::LEGACY_SIZE;

  if (settings.convertIntoLegacy) {
    int result = asDDS.ToLegacy(settings.convertIntoLegacyNonCannon);

    if (!result) {
      headerSize = DDS::LEGACY_SIZE;
    }
  }

  wr.WriteBuffer(reinterpret_cast<const char *>(&asDDS), headerSize);

  if (settings.noMips && asDDS.mipMapCount > 1) {
    asDDS.NumMipmaps(1);
    wr.WriteBuffer(buffer.data(), mips.sizes[0]);
  } else {
    wr.WriteContainer(buffer);
  }
}
