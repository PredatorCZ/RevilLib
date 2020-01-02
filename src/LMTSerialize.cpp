/*      Revil Format Library
        Copyright(C) 2017-2019 Lukas Cone

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

#include "LMTAnimation.h"
#include "LMTBoneTrack.h"
#include "LMTEvent.h"
#include "LMTFixupStorage.h"
#include "LMTFloatTrack.h"
#include "datas/binreader.hpp"
#include "datas/masterprinter.hpp"

bool IsX64CompatibleAnimationClass(BinReader &rd, short version);

int LMTAnimationEventV1_Internal::SaveBuffer(BinWritter *wr,
                                             LMTFixupStorage &fixups) const {
  const int numGroups = GetNumGroups();

  for (int g = 0; g < numGroups; g++) {
    wr->ApplyPadding();
    fixups.SaveTo(wr);

    wr->WriteContainer(GetEvents(g));
  }

  return 0;
}

int LMTAnimationEventV1_Internal::Save(BinWritter *wr) const {
  LMTFixupStorage localStorage;

  _Save(wr, localStorage);
  SaveBuffer(wr, localStorage);
  localStorage.FixupPointers(wr, _Is64bit());

  return 0;
}

int LMTAnimationEventV2_Internal::Save(BinWritter *wr) const {
  LMTFixupStorage localFixups;

  _Save(wr, localFixups);
  wr->ApplyPadding();
  localFixups.SaveTo(wr);

  for (auto &g : groups)
    g->_Save(wr, localFixups);

  for (auto &g : groups) {
    wr->ApplyPadding();
    localFixups.SaveTo(wr);

    LMTFixupStorage semilocalFixups;

    for (auto &e : g->events)
      e->_Save(wr, semilocalFixups);

    for (auto &e : g->events) {
      wr->ApplyPadding();
      semilocalFixups.SaveTo(wr);
      wr->WriteContainer(e->frames);
    }

    semilocalFixups.FixupPointers(wr, true);
  }

  localFixups.FixupPointers(wr, true);

  return 0;
}

int LMTTrack_internal::SaveBuffers(BinWritter *wr,
                                   LMTFixupStorage &storage) const {
  if (controller->NumFrames()) {
    wr->ApplyPadding();
    storage.SaveTo(wr);
    controller->Save(wr);
  } else
    storage.SkipTo();

  if (minMax) {
    wr->ApplyPadding();
    storage.SaveTo(wr);
    wr->Write(*minMax);
  }

  if (!minMax && UseTrackExtremes())
    storage.SkipTo();

  return 0;
}

int LMTFloatTrack_internal::Save(BinWritter *wr) const {
  LMTFixupStorage localStorage;

  _Save(wr, localStorage);
  wr->ApplyPadding();

  for (auto &f : frames) {
    if (f.size()) {
      localStorage.SaveTo(wr);
      wr->WriteContainer(f);
    } else
      localStorage.SkipTo();
  }

  localStorage.FixupPointers(wr, _Is64bit());

  return 0;
}

int LMTAnimation::Save(const char *fileName, bool supressErrors) const {

  BinWritter wr(fileName);

  if (!wr.IsValid()) {
    if (!supressErrors) {
      printerror("[LMT] Couldn't save file: ", << fileName);
    }
    return -1;
  }

  return Save(&wr);
}

int LMTAnimation_internal::Save(BinWritter *wr, bool standAlone) const {
  LMTFixupStorage fixups;
  size_t saveposBuffSize;

  if (standAlone) {
    wr->Write(MTMI);
    wr->Write<short>(props.version);
    saveposBuffSize = wr->Tell();
    wr->ApplyPadding();
  }

  _Save(wr, fixups);
  wr->ApplyPadding();
  fixups.SaveTo(wr);

  for (auto &t : tracks)
    reinterpret_cast<LMTTrack_internal *>(t.get())->Save(wr, fixups);

  const int aniVersion = GetVersion();

  if (aniVersion == 1)
    static_cast<LMTAnimationEventV1_Internal *>(events.get())
        ->SaveBuffer(wr, fixups);
  else if (aniVersion == 2) {
    LMTAnimationEventV1_Internal *cEvent =
        static_cast<LMTAnimationEventV1_Internal *>(events.get());

    if (cEvent) {
      wr->ApplyPadding();
      fixups.SaveTo(wr);
      cEvent->Save(wr);
    } else
      fixups.SkipTo();

    LMTFloatTrack_internal *cFloatTracks =
        static_cast<LMTFloatTrack_internal *>(floatTracks.get());

    if (cFloatTracks) {
      wr->ApplyPadding();
      fixups.SaveTo(wr);
      cFloatTracks->Save(wr);
    } else
      fixups.SkipTo();
  } else {
    LMTAnimationEventV2_Internal *cEvent =
        static_cast<LMTAnimationEventV2_Internal *>(events.get());

    if (cEvent) {
      wr->ApplyPadding();
      fixups.SaveTo(wr);
      cEvent->Save(wr);
    } else
      fixups.SkipTo();
  }

  for (auto &t : tracks)
    reinterpret_cast<LMTTrack_internal *>(t.get())->SaveBuffers(wr, fixups);

  if (standAlone) {
    fixups.SaveFrom(saveposBuffSize);
    fixups.SaveTo(wr);
  }

  fixups.FixupPointers(wr, _Is64bit());

  return 0;
}

int LMTAnimation_internal::Load(BinReader *rd,
                                LMTConstructorPropertiesBase expected,
                                LMTAnimation_internal *&out) {
  LMTConstructorProperties props;

  int magic;
  rd->Read(magic);

  if (magic != MTMI)
    return 1;

  rd->Read(reinterpret_cast<short &>(props));

  if (reinterpret_cast<short &>(props) != reinterpret_cast<short &>(expected))
    return reinterpret_cast<short &>(props);

  int bufferSize;
  rd->Read(bufferSize);

  if (!bufferSize)
    return 2;

  props.masterBuffer = static_cast<char *>(props.dataStart);
  props.dataStart = props.masterBuffer + rd->Tell();
  props.swappedEndian = rd->SwappedEndian();
  rd->Seek(0);
  rd->ReadBuffer(props.masterBuffer, bufferSize);

  out = static_cast<LMTAnimation_internal *>(LMTAnimation::Create(props));

  out->masterBuffer = MasterBufferPtr(props.masterBuffer);

  return 0;
}

int LMT::Load(BinReader *rd) {
  int magic;
  rd->Read(magic);

  if (magic == ID_R) {
    rd->SwapEndian(true);
  } else if (magic != ID) {
    printerror("[LMT] Invalid file.");
    return 1;
  }

  short version;
  rd->Read(version);

  if (!LMTAnimation::SupportedVersion(version)) {
    printerror("[LMT] Unknown version: ", << version);
    return 2;
  }

  short numBlocks;
  rd->Read(numBlocks);

  if (version == V_92)
    rd->Skip(8); // 0x17011700

  int calcutatedSizeX64 = numBlocks * 8 + static_cast<int>(rd->Tell());
  size_t padResult = calcutatedSizeX64 & 0xF;

  if (padResult)
    calcutatedSizeX64 += 16 - padResult;

  int calcutatedSizeX86 = numBlocks * 8 + static_cast<int>(rd->Tell());
  padResult = calcutatedSizeX86 & 0xF;

  if (padResult)
    calcutatedSizeX86 += 16 - padResult;

  magic = 0;

  while (!magic) {
    if (rd->GetStream()->eof())
      return 3;

    rd->Read(magic);
  }

  rd->Seek(magic);

  const bool isX64 = calcutatedSizeX64 != calcutatedSizeX86
              ? magic == calcutatedSizeX64
              : IsX64CompatibleAnimationClass(*rd, version);

  rd->Seek(0);

  const size_t fleSize = rd->GetSize();
  const int multiplier = isX64 ? 2 : 1;
  const int lookupTableOffset = 8 + (version == V_92 ? (4 * multiplier) : 0);

  Version(static_cast<V>(version), isX64 ? X64 : X86);

  masterBuffer = static_cast<char *>(malloc(fleSize));
  rd->ReadBuffer(masterBuffer, fleSize);

  int *lookupTable = reinterpret_cast<int *>(masterBuffer + lookupTableOffset);

  animations.resize(numBlocks);

  LMTConstructorProperties cProps;
  cProps = props;
  cProps.masterBuffer = masterBuffer;
  cProps.swappedEndian = rd->SwappedEndian();

  for (int a = 0; a < numBlocks; a++) {
    int &cOffset = *(lookupTable + (a * multiplier));

    if (rd->SwappedEndian()) {
      if (isX64)
        FByteswapper(reinterpret_cast<int64 &>(cOffset));
      else
        FByteswapper(cOffset);
    }

    if (!cOffset)
      continue;

    cProps.dataStart = masterBuffer + cOffset;

    animations[a] = LMTAnimation::Create(cProps);
  }

  return 0;
}

int LMT::Load(const char *fileName, bool supressErrors) {
  BinReader rd(fileName);

  if (!rd.IsValid()) {
    if (!supressErrors) {
      printerror("[LMT] Cannot open file: ", << fileName);
    }
    return -1;
  }

  return Load(&rd);
}

int LMT::Save(const char *fileName, bool swapEndian, bool supressErrors) const {
  BinWritter wr(fileName);

  if (!wr.IsValid()) {
    if (!supressErrors) {
      printerror("[LMT] Cannot save file: ", << fileName);
    }
    return -1;
  }

  wr.SwapEndian(swapEndian);

  return Save(&wr);
}

int LMT::Save(BinWritter *wr) const {
  wr->Write(ID);
  wr->Write<ushort>(Version());
  wr->Write<ushort>(animations.size());

  V eVersion = static_cast<V>(Version());
  bool isX64 = GetArchitecture() == X64;
  LMTFixupStorage fixups;

  if (eVersion == V_92) {
    wr->Write(0x17011700);
    wr->Write(0);
  }

  for (auto &a : animations) {
    fixups.SaveFrom(wr->Tell());
    wr->Skip(isX64 ? 8 : 4);
  }

  for (auto &a : animations) {
    if (!a) {
      fixups.SkipTo();
      continue;
    }

    wr->ApplyPadding();
    fixups.SaveTo(wr);
    a->Save(wr, false);
  }

  fixups.FixupPointers(wr, isX64);

  return 0;
}
