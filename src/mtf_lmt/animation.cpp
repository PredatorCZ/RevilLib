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
#include "datas/binreader_stream.hpp"
#include "datas/flags.hpp"
#include "datas/reflector.hpp"
#include "event.hpp"
#include "float_track.hpp"

MAKE_ENUM(ENUMSCOPE(class AnimV2Flags
                    : uint32, AnimV2Flags),
          EMEMBERVAL(Events, 0x800000), EMEMBERVAL(FLoatTracks, 0x40000),
          EMEMBERVAL(Unk00, 0x1000000), EMEMBERVAL(Unk01, 0x1))

#include "animation.inl"

static const ::LMTVersion supportedVersions[] = {
    LMT22, LMT40, LMT49, LMT50, LMT51, LMT56, LMT57, LMT66, LMT67, LMT92, LMT95,
};

bool LMTAnimation::SupportedVersion(uint16 version) {
  for (auto &v : supportedVersions) {
    if (static_cast<uint16>(v) == version) {
      return true;
    }
  }

  return false;
}

bool IsX64CompatibleAnimationClass(BinReaderRef_e rd, revil::LMTVersion version) {
  auto &layouts = clgen::Animation::LAYOUTS;
  clgen::LayoutLookup x64Layout{static_cast<uint8>(version), true, false};
  auto item = std::find_if(layouts.begin(), layouts.end(),
                           [&](auto &item) { return item == x64Layout; });

  if (es::IsEnd(layouts, item)) {
    return false;
  }

  char buffer[0x100];
  rd.ReadBuffer(buffer, sizeof(buffer));

  clgen::Animation::Interface interface(
      buffer, clgen::LayoutLookup{static_cast<uint8>(version), true, false});

  auto CheckPtr = [](char *data) {
    auto ptr = reinterpret_cast<uint64 *>(data);

    return !(*ptr & (0xffffffffULL << 32));
  };

  return [](auto... bls) { return (... && bls); }(
             CheckPtr(interface.TracksPtr().data),
             [&] {
               if (interface.LayoutVersion() >= LMT66) {
                 return CheckPtr(interface.EventsPtr().data);
               }
               return true;
             }(),
             [&] {
               if (interface.m(clgen::Animation::floats) >= 0) {
                 return CheckPtr(interface.FloatsPtr().data);
               }
               return true;
             }());
}

struct LMTAnimationMidInterface : LMTAnimationInterface {
  clgen::Animation::Interface interface;
  std::unique_ptr<LMTAnimationEvent> events;

  LMTAnimationMidInterface(clgen::LayoutLookup rules, char *data) : interface {
    data, rules
  } {
  }

  uni::MotionTracksConst Tracks() const override {
    return uni::MotionTracksConst(this, false);
  }

  std::string Name() const override { return ""; }
  void FrameRate(uint32 fps) const override {
    for (auto &t : storage) {
      static_cast<LMTTrackInterface &>(*t).frameRate = fps;
    }

    if (events) {
      static_cast<LMTAnimationEventInterface *>(events.get())->frameRate = fps;
    }
  }
  uint32 FrameRate() const override {
    return static_cast<LMTTrackInterface &>(*storage.front()).frameRate;
  }
  float Duration() const override {
    return static_cast<float>(NumFrames()) / static_cast<float>(FrameRate());
  }
  MotionType_e MotionType() const override { return MotionType_e::Relative; }

  size_t GetVersion() const override { return interface.lookup.version; }
  size_t NumFrames() const override { return interface.NumFrames(); }
  int32 LoopFrame() const override { return interface.LoopFrame(); }
  bool Is64bit() const override { return interface.lookup.x64; }
  const LMTAnimationEvent *Events() const override { return events.get(); }
};

template <>
void ProcessClass(LMTAnimationMidInterface &item,
                  LMTConstructorProperties flags) {
  size_t trackStride = 0;

  if (item.interface.TracksPtr().Check(flags.ptrStore)) {
    return;
  }

  item.interface.TracksPtr().Fixup(flags.base, flags.ptrStore);
  item.interface.FloatsPtr().Fixup(flags.base, flags.ptrStore);

  if (item.interface.LayoutVersion() >= LMT66) {
    item.interface.EventsPtr().Fixup(flags.base, flags.ptrStore);
    auto events = item.interface.EventsLMT66();
    if (events.data) {
      flags.dataStart = events.data;
      item.events = LMTAnimationEvent::Create(flags);
    }
  } else {
    auto events = item.interface.Events();
    flags.dataStart = events.data;
    item.events = LMTAnimationEvent::Create(flags);
  }

  for (size_t t = 0; t < item.interface.NumTracks(); t++) {
    flags.dataStart = item.interface.Tracks() + trackStride * t;
    auto track = LMTTrack::Create(flags);
    ProcessClass(*static_cast<LMTTrackInterface *>(track.get()), flags);
    trackStride = track->Stride();
    item.storage.emplace_back(std::move(track));
  }
}

using ptr_type_ = std::unique_ptr<LMTAnimation>;

ptr_type_ LMTAnimation::Create(const LMTConstructorProperties &props) {
  auto interface = std::make_unique<LMTAnimationMidInterface>(
      clgen::LayoutLookup{static_cast<uint8>(props.version),
                          props.arch == LMTArchType::X64, false},
      static_cast<char *>(props.dataStart));
  ProcessClass(*interface.get(), props);

  return interface;
}
