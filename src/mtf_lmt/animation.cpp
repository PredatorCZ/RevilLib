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
#include "datas/reflector_xml.hpp"
#include "event.hpp"
#include "fixup_storage.hpp"

#include <list>
#include <map>

/************************************************************************/
/*************************** ANIM_TRAITS ********************************/
/************************************************************************/

template <template <class C> class PtrType, LMTVersion trackVersion>
struct AnimTraitsV1 {
  using Track_Pointer = PtrType<char>;
  using EventClass = AnimEvents<PtrType>;

  static constexpr size_t NUMEVENTGROUPS = 2;
  static constexpr LMTVersion TRACK_VERSION = trackVersion;
};

template <template <class C> class PtrType, LMTVersion trackVersion>
struct AnimTraitsV2 {
  using Track_Pointer = PtrType<char>;
  using EventClass = AnimEvents<PtrType>;
  using EventClass_Pointer = PtrType<EventClass>;
  using FloatTracks_Pointer = PtrType<char>;

  static constexpr size_t NUMEVENTGROUPS = 4;
  static constexpr LMTVersion TRACK_VERSION = trackVersion;
};

/************************************************************************/
/***************************** ANIM_V0 **********************************/
/************************************************************************/

template <class AnimTraits>
struct AnimV0 : ReflectorInterface<AnimV0<AnimTraits>> {
  using Traits = AnimTraits;

  static constexpr size_t VERSION = 1;
  static constexpr size_t _EVENT_STRIDE = sizeof(typename Traits::EventClass);
  static constexpr size_t NUMPOINTERS = 1 + Traits::NUMEVENTGROUPS;

  typename Traits::Track_Pointer tracks;
  uint32 numTracks, numFrames;
  int32 loopFrame;
  Vector4A16 endFrameAdditiveScenePosition;
  typename Traits::EventClass events[Traits::NUMEVENTGROUPS];

  AnimV0() : numTracks(0), numFrames(0), loopFrame(0), events() {}

  auto Events() { return events; }

  char *FloatTracks() { return nullptr; }

  void SwapEndian() {
    FByteswapper(tracks);
    FByteswapper(numTracks);
    FByteswapper(numFrames);
    FByteswapper(loopFrame);
    FByteswapper(endFrameAdditiveScenePosition);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    if (tracks.Fixed()) {
      return;
    }

    if (swapEndian) {
      SwapEndian();
    }

    tracks.Fixup(masterBuffer, true);

    for (auto &e : events) {
      e.Fixup(masterBuffer, swapEndian);
    }
  }

  static const size_t *Pointers() {
    static const size_t ptrs[]{
        offsetof(AnimV0, tracks),           offsetof(AnimV0, events[0].events),
        offsetof(AnimV0, events[1].events), offsetof(AnimV0, events[2].events),
        offsetof(AnimV0, events[3].events),
    };

    return ptrs;
  }
};

/************************************************************************/
/***************************** ANIM_V1 **********************************/
/************************************************************************/

template <class AnimTraits>
struct AnimV1 : ReflectorInterface<AnimV1<AnimTraits>> {
  using Traits = AnimTraits;

  static constexpr size_t VERSION = 1;
  static constexpr size_t _EVENT_STRIDE = sizeof(typename Traits::EventClass);
  static constexpr size_t NUMPOINTERS = 1 + Traits::NUMEVENTGROUPS;

  typename Traits::Track_Pointer tracks;
  uint32 numTracks, numFrames;
  int32 loopFrame;
  Vector4A16 endFrameAdditiveScenePosition;
  Vector4A16 endFrameAdditiveSceneRotation;
  typename Traits::EventClass events[Traits::NUMEVENTGROUPS];

  auto Events() { return events; }

  char *FloatTracks() { return nullptr; }

  void SwapEndian() {
    FByteswapper(tracks);
    FByteswapper(numTracks);
    FByteswapper(numFrames);
    FByteswapper(loopFrame);
    FByteswapper(endFrameAdditiveScenePosition);
    FByteswapper(endFrameAdditiveSceneRotation);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    if (tracks.Fixed()) {
      return;
    }

    if (swapEndian) {
      SwapEndian();
    }

    tracks.Fixup(masterBuffer, true);

    for (auto &e : events) {
      e.Fixup(masterBuffer, swapEndian);
    }
  }

  static const size_t *Pointers() {
    static const size_t ptrs[]{
        offsetof(AnimV1, tracks),           offsetof(AnimV1, events[0].events),
        offsetof(AnimV1, events[1].events), offsetof(AnimV1, events[2].events),
        offsetof(AnimV1, events[3].events),
    };

    return ptrs;
  }
};

/************************************************************************/
/***************************** ANIM_V2 **********************************/
/************************************************************************/

REFLECTOR_CREATE(AnimV2Flags, ENUM, 2, CLASS, 32, Events = 0x800000,
                 FLoatTracks = 0x40000, Unk00 = 0x1000000, Unk01 = 0x1)

template <class AnimTraits>
struct AnimV2 : ReflectorInterface<AnimV2<AnimTraits>> {
  using Traits = AnimTraits;

  static constexpr size_t VERSION = 2;
  static constexpr size_t NUMPOINTERS = 3;

  typename Traits::Track_Pointer tracks;
  uint32 numTracks, numFrames;
  int32 loopFrame;
  Vector4A16 endFrameAdditiveScenePosition;
  Vector4A16 endFrameAdditiveSceneRotation;
  es::Flags<AnimV2Flags> flags;
  typename Traits::EventClass_Pointer eventTable;
  typename Traits::FloatTracks_Pointer floatTracks;

  auto Events() { return eventTable; }

  char *FloatTracks() { return floatTracks; }

  void SwapEndian() {
    FByteswapper(tracks);
    FByteswapper(numTracks);
    FByteswapper(numFrames);
    FByteswapper(loopFrame);
    FByteswapper(endFrameAdditiveScenePosition);
    FByteswapper(endFrameAdditiveSceneRotation);
    FByteswapper(flags);
    FByteswapper(eventTable);
    FByteswapper(floatTracks);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    if (tracks.Fixed()) {
      return;
    }

    if (swapEndian) {
      SwapEndian();
    }

    tracks.Fixup(masterBuffer, true);
    eventTable.Fixup(masterBuffer, true);
    floatTracks.Fixup(masterBuffer, true);

    auto dEvents = Events();

    if (dEvents) {
      for (size_t e = 0; e < Traits::NUMEVENTGROUPS; e++) {
        dEvents[e].Fixup(masterBuffer, swapEndian);
      }
    }
  }

  static const size_t *Pointers() {
    static const size_t ptrs[]{
        offsetof(AnimV2, tracks), offsetof(AnimV2, eventTable),
        offsetof(AnimV2, floatTracks),
    };

    return ptrs;
  }
};

/************************************************************************/
/***************************** ANIM_V3 **********************************/
/************************************************************************/

template <class AnimTraits>
struct AnimV3 : ReflectorInterface<AnimV3<AnimTraits>> {
  using Traits = AnimTraits;

  static constexpr size_t VERSION = 3;
  static constexpr size_t NUMPOINTERS = 2;

  typename Traits::Track_Pointer tracks;
  uint32 numTracks, numFrames;
  int32 loopFrame;
  Vector4A16 endFrameAdditiveScenePosition;
  Vector4A16 endFrameAdditiveSceneRotation;
  uint32 flags;
  typename Traits::Track_Pointer nullPtr[2];
  typename Traits::EventClass_Pointer eventTable;

  auto Events() { return eventTable; }

  char *FloatTracks() { return nullptr; }

  void SwapEndian() {
    FByteswapper(tracks);
    FByteswapper(numTracks);
    FByteswapper(numFrames);
    FByteswapper(loopFrame);
    FByteswapper(endFrameAdditiveScenePosition);
    FByteswapper(endFrameAdditiveSceneRotation);
    FByteswapper(eventTable);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    if (tracks.Fixed()) {
      return;
    }

    if (swapEndian) {
      SwapEndian();
    }

    tracks.Fixup(masterBuffer, true);
    eventTable.Fixup(masterBuffer, true);
  }

  static const size_t *Pointers() {
    static const size_t ptrs[]{
        offsetof(AnimV3, tracks), offsetof(AnimV3, eventTable),
    };

    return ptrs;
  }
};

/************************************************************************/
/************************** ANIM_INTERNAL *******************************/
/************************************************************************/

void LMTAnimation_internal::FrameRate(uint32 fps) const {
  for (auto &s : storage) {
    static_cast<LMTTrack_internal *>(s.get())->frameRate =
        static_cast<float>(fps);
  }
}
uint32 LMTAnimation_internal::FrameRate() const {
  return static_cast<uint32>(
      static_cast<const LMTTrack_internal *>(storage.at(0).get())->frameRate);
}

float LMTAnimation_internal::Duration() const {
  return static_cast<float>(NumFrames()) / static_cast<float>(FrameRate());
}

template <class C> class AnimShared : public LMTAnimation_internal {
  static constexpr LMTArchType GetArch() {
    return sizeof(typename C::Traits::Track_Pointer) == 8 ? LMTArchType::X64
                                                          : LMTArchType::X86;
  }

public:
  using value_type = C;

  mutable uni::Element<value_type> data;

  AnimShared() : data(new C) {}
  AnimShared(C *ptr, char *buff, bool swapEndian) : data(ptr, false) {
    if (!buff) {
      if (swapEndian) {
        data->SwapEndian();
      }

      return;
    }

    data->Fixup(buff, swapEndian);

    if (data->numTracks) {
      LMTConstructorProperties props;
      props.arch = GetArch();
      props.version = C::Traits::TRACK_VERSION;
      props.masterBuffer = buff;
      props.swappedEndian = swapEndian;
      props.dataStart = data->tracks;

      size_t curTrack = 0;
      do {
        storage.emplace_back(LMTTrack::Create(props));
        const auto trackStride = storage.back()->Stride();
        props.dataStart = data->tracks + trackStride;
      } while (++curTrack < data->numTracks);
    }

    LMTConstructorProperties eventProps;
    eventProps.arch = GetArch();
    eventProps.swappedEndian = swapEndian;
    eventProps.dataStart = data->Events();
    eventProps.masterBuffer = buff;

    if (C::VERSION < 3) {
      eventProps.version =
          C::Traits::NUMEVENTGROUPS > 2 ? LMTVersion::V_56 : LMTVersion::V_22;
    } else {
      eventProps.version = LMTVersion::V_66;
    }

    events = LMTAnimationEvent::Create(eventProps);

    eventProps.dataStart = data->FloatTracks();
    eventProps.version = LMTVersion::Auto;

    if (eventProps.dataStart) {
      floatTracks = LMTFloatTrack::Create(eventProps);
    }
  }

  void ReflectToXML(pugi::xml_node &node) const override {
    ReflectorWrapConst<C> refl(data.get());
    ReflectorXMLUtil::Save(refl, node);
  }

  void ReflectFromXML(pugi::xml_node &node) override {
    ReflectorWrap<C> refl(data.get());
    ReflectorXMLUtil::Load(refl, node);
  }

  std::vector<uint64> GetPtrValues() const override {
    std::vector<uint64> retVal;
    constexpr size_t numPoninters = C::NUMPOINTERS;
    const auto *pointers = C::Pointers();

    for (size_t p = 0; p < numPoninters; p++) {
      retVal.push_back(*reinterpret_cast<const uint64 *>(
          reinterpret_cast<const char *>(data.get()) + pointers[p]));
    }

    return retVal;
  }

  LMTTrackPtr CreateTrack() const override {
    LMTConstructorProperties props;
    props.arch = GetArch();
    props.version = C::Traits::TRACK_VERSION;

    return LMTTrack::Create(props);
  }

  LMTEventsPtr CreateEvents() const override {
    LMTConstructorProperties props;
    props.arch = GetArch();

    if (C::VERSION < 3) {
      props.version =
          C::Traits::NUMEVENTGROUPS > 2 ? LMTVersion::V_56 : LMTVersion::V_22;
    } else {
      props.version = LMTVersion::V_66;
    }

    return LMTAnimationEvent::Create(props);
  }

  LMTFloatTrackPtr CreateFloatTracks() const override {
    if (C::VERSION != 2) {
      return {};
    }

    LMTConstructorProperties props;
    props.arch = GetArch();

    return LMTFloatTrack::Create(props);
  }

  void SaveInternal(BinWritterRef wr, LMTFixupStorage &storage) const override {
    const size_t cOff = wr.Tell();
    constexpr size_t numPoninters = C::NUMPOINTERS;
    const auto *pointers = C::Pointers();

    wr.Write<value_type &>(*data);

    for (size_t p = 0; p < numPoninters; p++) {
      storage.SaveFrom(cOff + pointers[p]);
    }
  }

  size_t GetVersion() const override { return C::VERSION; }

  bool Is64bit() const override {
    return sizeof(typename C::Traits::Track_Pointer) == 8;
  }

  size_t NumFrames() const override { return data->numFrames; }

  int32 LoopFrame() const override { return data->loopFrame; }

  // finish
  void Sanitize() const {
    data->numTracks = static_cast<uint32>(storage.size());

    if (!data->numTracks)
      return;

    int32 maxFrame = 0;

    for (auto &t : storage) {
      if (!t->NumFrames())
        continue;

      int32 curEndFrame = t->GetFrame(t->NumFrames() - 1);

      if (curEndFrame > maxFrame)
        maxFrame = curEndFrame;
    }

    data->numFrames =
        maxFrame +
        static_cast<const LMTTrack_internal *>(storage[0].get())->useRefFrame;
  }
};

REFLECTOR_CREATE((AnimV0<AnimTraitsV1<esPointerX86, LMTVersion::V_22>>), 2,
                 VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition);

REFLECTOR_CREATE((AnimV0<AnimTraitsV1<esPointerX64, LMTVersion::V_22>>), 2,
                 VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition);

REFLECTOR_CREATE((AnimV1<AnimTraitsV1<esPointerX86, LMTVersion::V_40>>), 2,
                 VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

REFLECTOR_CREATE((AnimV1<AnimTraitsV1<esPointerX64, LMTVersion::V_40>>), 2,
                 VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

REFLECTOR_CREATE((AnimV1<AnimTraitsV1<esPointerX86, LMTVersion::V_51>>), 2,
                 VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

REFLECTOR_CREATE((AnimV1<AnimTraitsV1<esPointerX64, LMTVersion::V_51>>), 2,
                 VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

REFLECTOR_CREATE((AnimV1<AnimTraitsV2<esPointerX86, LMTVersion::V_56>>), 2,
                 VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

REFLECTOR_CREATE((AnimV1<AnimTraitsV2<esPointerX64, LMTVersion::V_56>>), 2,
                 VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

REFLECTOR_CREATE((AnimV2<AnimTraitsV2<esPointerX86, LMTVersion::V_56>>), 2,
                 VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

REFLECTOR_CREATE((AnimV2<AnimTraitsV2<esPointerX64, LMTVersion::V_56>>), 2,
                 VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

REFLECTOR_CREATE((AnimV3<AnimTraitsV2<esPointerX86, LMTVersion::V_92>>), 2,
                 VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

REFLECTOR_CREATE((AnimV3<AnimTraitsV2<esPointerX64, LMTVersion::V_92>>), 2,
                 VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

ES_STATIC_ASSERT(sizeof(AnimV1<AnimTraitsV1<esPointerX86, LMTVersion::V_51>>) ==
                 192);
ES_STATIC_ASSERT(sizeof(AnimV3<AnimTraitsV2<esPointerX64, LMTVersion::V_92>>) ==
                 96);
ES_STATIC_ASSERT(
    alignof(AnimV3<AnimTraitsV2<esPointerX64, LMTVersion::V_92>>) == 16);
// ES_STATIC_ASSERT(offsetof(AnimV3X64TrackV3, eventTable) == 88);

using ptr_type_ = LMTAnimation::Ptr;

template <class C> struct f_ {
  static ptr_type_ creatorBase() { return std::make_unique<AnimShared<C>>(); }

  static ptr_type_ creator(void *ptr, char *buff, bool endi) {
    return std::make_unique<AnimShared<C>>(static_cast<C *>(ptr), buff, endi);
  }
};

#define LMTANI_REG(version, ptrSize, ...)                                      \
  {                                                                            \
    {LMTArchType::X##ptrSize, LMTVersion::V_##version},                        \
        f_<__VA_ARGS__>::creatorBase                                           \
  }

#define LMTANI_REG_LINK(version, ptrSize, ...)                                 \
  {                                                                            \
    {LMTArchType::X##ptrSize, LMTVersion::V_##version},                        \
        f_<__VA_ARGS__>::creator                                               \
  }

static const std::map<LMTConstructorPropertiesBase,
                      decltype(&f_<void>::creatorBase)>
    animationRegistry = {
        // clang-format off
        LMTANI_REG(22, 86, AnimV0<AnimTraitsV1<esPointerX86, LMTVersion::V_22>>),
        LMTANI_REG(40, 86, AnimV1<AnimTraitsV1<esPointerX86, LMTVersion::V_40>>),
        LMTANI_REG(49, 86, AnimV1<AnimTraitsV1<esPointerX86, LMTVersion::V_40>>),
        LMTANI_REG(50, 86, AnimV1<AnimTraitsV1<esPointerX86, LMTVersion::V_40>>),
        LMTANI_REG(51, 86, AnimV1<AnimTraitsV1<esPointerX86, LMTVersion::V_51>>),
        LMTANI_REG(56, 86, AnimV1<AnimTraitsV2<esPointerX86, LMTVersion::V_56>>),
        LMTANI_REG(57, 86, AnimV1<AnimTraitsV2<esPointerX86, LMTVersion::V_56>>),
        LMTANI_REG(66, 86, AnimV2<AnimTraitsV2<esPointerX86, LMTVersion::V_56>>),
        LMTANI_REG(67, 86, AnimV2<AnimTraitsV2<esPointerX86, LMTVersion::V_56>>),
        LMTANI_REG(92, 86, AnimV3<AnimTraitsV2<esPointerX86, LMTVersion::V_92>>),

        LMTANI_REG(22, 64, AnimV0<AnimTraitsV1<esPointerX64, LMTVersion::V_22>>),
        LMTANI_REG(40, 64, AnimV1<AnimTraitsV1<esPointerX64, LMTVersion::V_40>>),
        LMTANI_REG(49, 64, AnimV1<AnimTraitsV1<esPointerX64, LMTVersion::V_40>>),
        LMTANI_REG(50, 64, AnimV1<AnimTraitsV1<esPointerX64, LMTVersion::V_40>>),
        LMTANI_REG(51, 64, AnimV1<AnimTraitsV1<esPointerX64, LMTVersion::V_51>>),
        LMTANI_REG(56, 64, AnimV1<AnimTraitsV2<esPointerX64, LMTVersion::V_56>>),
        LMTANI_REG(57, 64, AnimV1<AnimTraitsV2<esPointerX64, LMTVersion::V_56>>),
        LMTANI_REG(66, 64, AnimV2<AnimTraitsV2<esPointerX64, LMTVersion::V_56>>),
        LMTANI_REG(67, 64, AnimV2<AnimTraitsV2<esPointerX64, LMTVersion::V_56>>),
        LMTANI_REG(92, 64, AnimV3<AnimTraitsV2<esPointerX64, LMTVersion::V_92>>),
        // clang-format on
};

static const std::map<LMTConstructorPropertiesBase,
                      decltype(&f_<void>::creator)>
    animationLinkRegistry = {
        // clang-format off
        LMTANI_REG_LINK(22, 86, AnimV0<AnimTraitsV1<esPointerX86, LMTVersion::V_22>>),
        LMTANI_REG_LINK(40, 86, AnimV1<AnimTraitsV1<esPointerX86, LMTVersion::V_40>>),
        LMTANI_REG_LINK(49, 86, AnimV1<AnimTraitsV1<esPointerX86, LMTVersion::V_40>>),
        LMTANI_REG_LINK(50, 86, AnimV1<AnimTraitsV1<esPointerX86, LMTVersion::V_40>>),
        LMTANI_REG_LINK(51, 86, AnimV1<AnimTraitsV1<esPointerX86, LMTVersion::V_51>>),
        LMTANI_REG_LINK(56, 86, AnimV1<AnimTraitsV2<esPointerX86, LMTVersion::V_56>>),
        LMTANI_REG_LINK(57, 86, AnimV1<AnimTraitsV2<esPointerX86, LMTVersion::V_56>>),
        LMTANI_REG_LINK(66, 86, AnimV2<AnimTraitsV2<esPointerX86, LMTVersion::V_56>>),
        LMTANI_REG_LINK(67, 86, AnimV2<AnimTraitsV2<esPointerX86, LMTVersion::V_56>>),
        LMTANI_REG_LINK(92, 86, AnimV3<AnimTraitsV2<esPointerX86, LMTVersion::V_92>>),

        LMTANI_REG_LINK(22, 64, AnimV0<AnimTraitsV1<esPointerX64, LMTVersion::V_22>>),
        LMTANI_REG_LINK(40, 64, AnimV1<AnimTraitsV1<esPointerX64, LMTVersion::V_40>>),
        LMTANI_REG_LINK(49, 64, AnimV1<AnimTraitsV1<esPointerX64, LMTVersion::V_40>>),
        LMTANI_REG_LINK(50, 64, AnimV1<AnimTraitsV1<esPointerX64, LMTVersion::V_40>>),
        LMTANI_REG_LINK(51, 64, AnimV1<AnimTraitsV1<esPointerX64, LMTVersion::V_51>>),
        LMTANI_REG_LINK(56, 64, AnimV1<AnimTraitsV2<esPointerX64, LMTVersion::V_56>>),
        LMTANI_REG_LINK(57, 64, AnimV1<AnimTraitsV2<esPointerX64, LMTVersion::V_56>>),
        LMTANI_REG_LINK(66, 64, AnimV2<AnimTraitsV2<esPointerX64, LMTVersion::V_56>>),
        LMTANI_REG_LINK(67, 64, AnimV2<AnimTraitsV2<esPointerX64, LMTVersion::V_56>>),
        LMTANI_REG_LINK(92, 64, AnimV3<AnimTraitsV2<esPointerX64, LMTVersion::V_92>>),
        // clang-format on
};

static const std::list<LMTVersion> supportedVersions = {
    LMTVersion::V_22, LMTVersion::V_40, LMTVersion::V_49, LMTVersion::V_50,
    LMTVersion::V_51, LMTVersion::V_56, LMTVersion::V_57, LMTVersion::V_66,
    LMTVersion::V_67, LMTVersion::V_92,
};

bool LMTAnimation::SupportedVersion(uint16 version) {
  for (auto &v : supportedVersions) {
    if (static_cast<uint16>(v) == version) {
      return true;
    }
  }

  return false;
}

ptr_type_ LMTAnimation::Create(const LMTConstructorProperties &props) {
  RegisterReflectedType<AnimV2Flags>();
  ptr_type_ cAni;

  if (props.dataStart) {
    cAni = animationLinkRegistry.at(props)(props.dataStart, props.masterBuffer,
                                           props.swappedEndian);
  } else {
    cAni = animationRegistry.at(props)();
  }

  cAni->props = props;

  return cAni;
}

bool IsX64CompatibleAnimationClass(BinReaderRef rd, LMTVersion version) {
  LMTConstructorPropertiesBase props{LMTArchType::X64, version};
  RegisterReflectedType<AnimV2Flags>();

  if (!animationRegistry.count(props)) {
    return false;
  }

  char buffer[0x100];

  rd.ReadBuffer(buffer, 0x100);

  auto anim =
      animationLinkRegistry.at(props)(buffer, nullptr, rd.SwappedEndian());
  auto cAni = static_cast<LMTAnimation_internal *>(anim.get());
  auto ptrVals = cAni->GetPtrValues();

  for (auto v : ptrVals) {
    if (v & (0xffffffffULL << 32)) {
      return false;
    }
  }

  return true;
}
