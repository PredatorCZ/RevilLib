/*  Revil Format Library
    Copyright(C) 2017-2023 Lukas Cone

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

#include "animation.hpp"
#include "bone_track.hpp"
#include "event.hpp"
#include "fixup_storage.hpp"
#include "float_track.hpp"
#include "spike/except.hpp"
#include "spike/io/binreader.hpp"
#include "spike/io/binwritter.hpp"

static constexpr uint32 MTMI = CompileFourCC("MTMI");
static constexpr uint32 LMT_ID = CompileFourCC("LMT\0");
static constexpr uint32 TML_ID = CompileFourCC("\0TML");

bool IsX64CompatibleAnimationClass(BinReaderRef_e rd, LMTVersion version);

LMTAnimation::Ptr
LMTAnimationInterface::Load(BinReaderRef_e rd,
                            LMTConstructorPropertiesBase expected) {
  LMTConstructorPropertiesBase props;

  uint32 magic;
  rd.Read(magic);

  if (magic != MTMI) {
    throw es::InvalidHeaderError(magic);
  }

  uint16 &version = reinterpret_cast<uint16 &>(props);

  rd.Read(version);

  if (props != expected) {
    std::string msg;

    if (props.version != expected.version) {
      msg += "Version mismatch [" +
             std::to_string(static_cast<int>(props.version)) + "], expected [" +
             std::to_string(static_cast<int>(expected.version)) + "]. ";
    }

    if (props.arch != expected.arch) {
      bool expectedX64Arch = expected.arch == LMTArchType::X64;
      bool haveX64Arch = props.arch == LMTArchType::X64;
      msg += "Architecture mismatch [" +
             std::string(haveX64Arch ? "X64" : "X86") + "], expected [" +
             std::string(expectedX64Arch ? "X64" : "X86") + "].";
    }

    throw std::runtime_error(msg);
  }

  uint32 bufferSize;
  rd.Read(bufferSize);

  if (!bufferSize) {
    throw std::runtime_error("Empty buffer.");
  }

  auto buff = std::make_unique<std::string>();
  rd.ApplyPadding();
  const size_t dataStart = rd.Tell();
  rd.Seek(0);
  rd.ReadContainer(*buff, bufferSize);

  std::vector<void *> ptrStore;
  LMTConstructorProperties cProps(props, ptrStore);

  cProps.base = buff.get()->data();
  cProps.dataStart = cProps.base + dataStart;
  cProps.swapEndian = rd.SwappedEndian();

  auto out = LMTAnimation::Create(cProps);
  static_cast<LMTAnimationInterface *>(out.get())->standAloneHolder =
      std::move(buff);

  return out;
}

void LMT::Load(BinReaderRef_e rd) {
  uint32 magic;
  rd.Read(magic);

  if (magic == TML_ID) {
    rd.SwapEndian(true);
  } else if (magic != LMT_ID) {
    throw es::InvalidHeaderError(magic);
  }

  uint16 iversion;
  rd.Read(iversion);
  LMTVersion version = static_cast<LMTVersion>(iversion);

  if (!LMTAnimation::SupportedVersion(iversion)) {
    throw es::InvalidVersionError(iversion);
  }

  uint16 numBlocks;
  rd.Read(numBlocks);

  if (!numBlocks) {
    return;
  }

  if (version >= LMTVersion::V_92) {
    rd.Skip(8); // 0x17011700 v92, 0x18020800 v95
  }

  size_t calcutatedSizeX64 = numBlocks * sizeof(uint64) + rd.Tell();
  calcutatedSizeX64 += GetPadding(calcutatedSizeX64, 16);

  size_t calcutatedSizeX86 = numBlocks * sizeof(uint32) + rd.Tell();
  calcutatedSizeX86 += GetPadding(calcutatedSizeX86, 16);

  magic = 0;

  while (!magic) {
    if (rd.IsEOF()) {
      return;
    }

    rd.Read(magic);
  }

  rd.Seek(magic);

  const bool isX64 = calcutatedSizeX64 != calcutatedSizeX86
                         ? magic == calcutatedSizeX64
                         : IsX64CompatibleAnimationClass(rd, version);

  rd.Seek(0);

  const size_t fleSize = rd.GetSize();
  const size_t multiplier = isX64 ? 2 : 1;
  const size_t lookupTableOffset =
      8 + (version >= LMTVersion::V_92 ? (4 * multiplier) : 0);

  Version(version, isX64 ? LMTArchType::X64 : LMTArchType::X86);

  rd.ReadContainer(pi->masterBuffer, fleSize);
  char *buffer = &pi->masterBuffer[0];

  uint32 *lookupTable = reinterpret_cast<uint32 *>(buffer + lookupTableOffset);

  pi->storage.resize(numBlocks);
  std::vector<void *> ptrStore;

  LMTConstructorProperties cProps(pi->props, ptrStore);
  cProps.base = buffer;
  cProps.swapEndian = rd.SwappedEndian();

  for (uint32 a = 0; a < numBlocks; a++) {
    uint32 &cOffset = *(lookupTable + (a * multiplier));

    if (rd.SwappedEndian()) {
      if (isX64) {
        FByteswapper(reinterpret_cast<int64 &>(cOffset));
      } else {
        FByteswapper(cOffset);
      }
    }

    if (!cOffset) {
      continue;
    }

    cProps.dataStart = buffer + cOffset;

    pi->storage[a] = uni::ToElement(LMTAnimation::Create(cProps));
  }
}

void LMT::Save(BinWritterRef wr) const {
  wr.Write(LMT_ID);
  wr.Write(static_cast<uint16>(Version()));
  wr.Write(static_cast<uint16>(pi->storage.size()));

  auto eVersion = Version();
  bool isX64 = Architecture() == LMTArchType::X64;
  LMTFixupStorage fixups;

  if (eVersion == LMTVersion::V_92) {
    wr.Write(0x17011700);
    wr.Write(0);
  } else if (eVersion == LMTVersion::V_95) {
    wr.Write(0x18020800);
    wr.Write(0);
  }

  for (auto &a : pi->storage) {
    fixups.SaveFrom(wr.Tell());
    wr.Skip(isX64 ? 8 : 4);
  }

  for (auto &a : pi->storage) {
    if (!a) {
      fixups.SkipTo();
      continue;
    }

    wr.ApplyPadding();
    fixups.SaveTo(wr);
    // a->Save(wr, false);
  }

  fixups.FixupPointers(wr, isX64);
}
