/*  Revil Format Library
    Copyright(C) 2017-2020 Lukas Cone

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
#include "datas/binreader.hpp"
#include "datas/binwritter.hpp"
#include "datas/except.hpp"
#include "event.hpp"
#include "fixup_storage.hpp"
#include "float_track.hpp"

static constexpr uint32 MTMI = CompileFourCC("MTMI");
static constexpr uint32 LMT_ID = CompileFourCC("LMT\0");
static constexpr uint32 TML_ID = CompileFourCC("\0TML");

bool IsX64CompatibleAnimationClass(BinReaderRef rd, LMTVersion version);

void LMTAnimationEventV1_Internal::SaveBuffer(BinWritterRef wr,
                                              LMTFixupStorage &fixups) const {
  const size_t numGroups = GetNumGroups();

  for (size_t g = 0; g < numGroups; g++) {
    wr.ApplyPadding();
    fixups.SaveTo(wr);

    wr.WriteContainer(GetEvents(g));
  }
}

void LMTAnimationEventV1_Internal::Save(BinWritterRef wr) const {
  LMTFixupStorage localStorage;

  SaveInternal(wr, localStorage);
  SaveBuffer(wr, localStorage);
  localStorage.FixupPointers(wr, Is64bit());
}

void LMTAnimationEventV2_Internal::Save(BinWritterRef wr) const {
  LMTFixupStorage localFixups;

  SaveInternal(wr, localFixups);
  wr.ApplyPadding();
  localFixups.SaveTo(wr);

  for (auto &g : groups) {
    g->SaveInternal(wr, localFixups);
  }

  for (auto &g : groups) {
    wr.ApplyPadding();
    localFixups.SaveTo(wr);

    LMTFixupStorage semilocalFixups;

    for (auto &e : g->events) {
      e->SaveInternal(wr, semilocalFixups);
    }

    for (auto &e : g->events) {
      wr.ApplyPadding();
      semilocalFixups.SaveTo(wr);
      wr.WriteContainer(e->frames);
    }

    semilocalFixups.FixupPointers(wr, true);
  }

  localFixups.FixupPointers(wr, true);
}

void LMTTrack_internal::SaveBuffers(BinWritterRef wr,
                                    LMTFixupStorage &storage) const {
  if (controller->NumFrames()) {
    wr.ApplyPadding();
    storage.SaveTo(wr);
    controller->Save(wr);
  } else {
    storage.SkipTo();
  }

  if (minMax) {
    wr.ApplyPadding();
    storage.SaveTo(wr);
    wr.Write<TrackMinMax &>(*minMax);
  }

  if (!minMax && UseTrackExtremes()) {
    storage.SkipTo();
  }
}

void LMTFloatTrack_internal::Save(BinWritterRef wr) const {
  LMTFixupStorage localStorage;

  SaveInternal(wr, localStorage);
  wr.ApplyPadding();

  for (auto &f : frames) {
    if (f.size()) {
      localStorage.SaveTo(wr);
      wr.WriteContainer(f);
    } else {
      localStorage.SkipTo();
    }
  }

  localStorage.FixupPointers(wr, Is64bit());
}

void LMTAnimation_internal::Save(BinWritterRef wr, bool standAlone) const {
  LMTFixupStorage fixups;
  size_t saveposBuffSize;

  if (standAlone) {
    wr.Write(MTMI);
    wr.Write(reinterpret_cast<const uint16 &>(props));
    saveposBuffSize = wr.Tell();
    wr.ApplyPadding();
  }

  SaveInternal(wr, fixups);
  wr.ApplyPadding();
  fixups.SaveTo(wr);

  for (auto &t : storage) {
    static_cast<LMTTrack_internal *>(t.get())->SaveInternal(wr, fixups);
  }

  const size_t aniVersion = GetVersion();

  if (aniVersion == 1) {
    static_cast<LMTAnimationEventV1_Internal *>(events.get())
        ->SaveBuffer(wr, fixups);
  } else if (aniVersion == 2) {
    auto cEvent = static_cast<LMTAnimationEventV1_Internal *>(events.get());

    if (cEvent) {
      wr.ApplyPadding();
      fixups.SaveTo(wr);
      cEvent->Save(wr);
    } else {
      fixups.SkipTo();
    }

    auto cFloatTracks =
        static_cast<LMTFloatTrack_internal *>(floatTracks.get());

    if (cFloatTracks) {
      wr.ApplyPadding();
      fixups.SaveTo(wr);
      cFloatTracks->Save(wr);
    } else {
      fixups.SkipTo();
    }
  } else {
    auto cEvent = static_cast<LMTAnimationEventV2_Internal *>(events.get());

    if (cEvent) {
      wr.ApplyPadding();
      fixups.SaveTo(wr);
      cEvent->Save(wr);
    } else {
      fixups.SkipTo();
    }
  }

  for (auto &t : storage) {
    static_cast<LMTTrack_internal *>(t.get())->SaveBuffers(wr, fixups);
  }

  if (standAlone) {
    fixups.SaveFrom(saveposBuffSize);
    fixups.SaveTo(wr);
  }

  fixups.FixupPointers(wr, Is64bit());
}

LMTAnimation::Ptr
LMTAnimation_internal::Load(BinReaderRef rd,
                            LMTConstructorPropertiesBase expected) {
  LMTConstructorProperties props;

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

  props.masterBuffer = &(*buff.get())[0];
  props.dataStart = props.masterBuffer + dataStart;
  props.swappedEndian = rd.SwappedEndian();

  auto out = LMTAnimation::Create(props);
  static_cast<LMTAnimation_internal *>(out.get())->masterBuffer =
      uni::ToElement(buff);

  ClearESPointers();

  return out;
}

void LMT::Load(BinReaderRef rd) {
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
    throw es::InvalidVersionError(magic);
  }

  uint16 numBlocks;
  rd.Read(numBlocks);

  if (!numBlocks) {
    return;
  }

  if (version == LMTVersion::V_92) {
    rd.Skip(8); // 0x17011700
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
      8 + (version == LMTVersion::V_92 ? (4 * multiplier) : 0);

  Version(version, isX64 ? LMTArchType::X64 : LMTArchType::X86);

  rd.ReadContainer(masterBuffer, fleSize);
  char *buffer = &masterBuffer[0];

  uint32 *lookupTable = reinterpret_cast<uint32 *>(buffer + lookupTableOffset);

  storage.resize(numBlocks);

  LMTConstructorProperties cProps;
  cProps = props;
  cProps.masterBuffer = buffer;
  cProps.swappedEndian = rd.SwappedEndian();

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

    storage[a] = uni::ToElement(LMTAnimation::Create(cProps));
  }

  ClearESPointers();
}

void LMT::Save(BinWritterRef wr) const {
  wr.Write(LMT_ID);
  wr.Write(static_cast<uint16>(Version()));
  wr.Write(static_cast<uint16>(storage.size()));

  auto eVersion = Version();
  bool isX64 = Architecture() == LMTArchType::X64;
  LMTFixupStorage fixups;

  if (eVersion == LMTVersion::V_92) {
    wr.Write(0x17011700);
    wr.Write(0);
  }

  for (auto &a : storage) {
    fixups.SaveFrom(wr.Tell());
    wr.Skip(isX64 ? 8 : 4);
  }

  for (auto &a : storage) {
    if (!a) {
      fixups.SkipTo();
      continue;
    }

    wr.ApplyPadding();
    fixups.SaveTo(wr);
    a->Save(wr, false);
  }

  fixups.FixupPointers(wr, isX64);
}
