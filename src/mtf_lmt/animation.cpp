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
#include <unordered_map>

/************************************************************************/
/*************************** ANIM_TRAITS ********************************/
/************************************************************************/

template <template <class C> class PtrType, const uint32 trackVersion>
struct AnimTraitsV1 {
  typedef PtrType<char> Track_Pointer;
  typedef AnimEvents<PtrType> EventClass;

  static const uint32 NUMEVENTGROUPS = 2;
  static const uint32 TRACK_VERSION = trackVersion;
};

template <template <class C> class PtrType, const uint32 trackVersion>
struct AnimTraitsV2 {
  typedef PtrType<char> Track_Pointer;
  typedef AnimEvents<PtrType> EventClass;
  typedef PtrType<EventClass> EventClass_Pointer;
  typedef PtrType<char> FloatTracks_Pointer;

  static const uint32 NUMEVENTGROUPS = 4;
  static const uint32 TRACK_VERSION = trackVersion;
};

/************************************************************************/
/***************************** ANIM_V0 **********************************/
/************************************************************************/

template <class AnimTraits> struct AnimV0 {
  DECLARE_REFLECTOR;
  typedef AnimTraits Traits;

  typename Traits::Track_Pointer tracks;
  uint32 numTracks, numFrames;
  int32 loopFrame;
  Vector4A16 endFrameAdditiveScenePosition;
  typename Traits::EventClass events[Traits::NUMEVENTGROUPS];

  AnimV0() : numTracks(0), numFrames(0), loopFrame(0), events() {}

  static const uint32 VERSION = 1;
  static const uint32 _EVENT_STRIDE = sizeof(typename Traits::EventClass);
  static const uint32 POINTERS[];

  typename Traits::EventClass *Events(char *masterBuffer) { return events; }

  char *FloatTracks(char *) { return nullptr; }

  void SwapEndian() {
    FByteswapper(numTracks);
    FByteswapper(numFrames);
    FByteswapper(loopFrame);
    FByteswapper(endFrameAdditiveScenePosition);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    tracks.Fixup(masterBuffer, swapEndian);

    for (auto &e : events)
      e.Fixup(masterBuffer, swapEndian);

    if (swapEndian)
      SwapEndian();
  }
};

/************************************************************************/
/***************************** ANIM_V1 **********************************/
/************************************************************************/

template <class AnimTraits> struct AnimV1 {
  DECLARE_REFLECTOR;
  typedef AnimTraits Traits;

  typename Traits::Track_Pointer tracks;
  uint32 numTracks, numFrames;
  int32 loopFrame;
  Vector4A16 endFrameAdditiveScenePosition;
  Vector4A16 endFrameAdditiveSceneRotation;
  typename Traits::EventClass events[Traits::NUMEVENTGROUPS];

  static const uint32 VERSION = 1;
  static const uint32 _EVENT_STRIDE = sizeof(typename Traits::EventClass);
  static const uint32 POINTERS[];

  typename Traits::EventClass *Events(char *masterBuffer) { return events; }

  char *FloatTracks(char *) { return nullptr; }

  void SwapEndian() {
    FByteswapper(numTracks);
    FByteswapper(numFrames);
    FByteswapper(loopFrame);
    FByteswapper(endFrameAdditiveScenePosition);
    FByteswapper(endFrameAdditiveSceneRotation);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    tracks.Fixup(masterBuffer, swapEndian);

    for (auto &e : events)
      e.Fixup(masterBuffer, swapEndian);

    if (swapEndian)
      SwapEndian();
  }
};

/************************************************************************/
/***************************** ANIM_V2 **********************************/
/************************************************************************/

REFLECTOR_CREATE(AnimV2Flags, ENUM, 2, CLASS, 32, Events = 0x800000,
                 FLoatTracks = 0x40000, Unk00 = 0x1000000, Unk01 = 0x1)

template <class AnimTraits> struct AnimV2 {
  DECLARE_REFLECTOR;
  typedef AnimTraits Traits;

  typename Traits::Track_Pointer tracks;
  uint32 numTracks, numFrames;
  int32 loopFrame;
  Vector4A16 endFrameAdditiveScenePosition;
  Vector4A16 endFrameAdditiveSceneRotation;
  esFlags<uint32, AnimV2Flags> flags;
  typename Traits::EventClass_Pointer eventTable;
  typename Traits::FloatTracks_Pointer floatTracks;

  typename Traits::EventClass *Events(char *masterBuffer) {
    return eventTable.GetData(masterBuffer);
  }

  char *FloatTracks(char *masterBuffer) {
    return floatTracks.GetData(masterBuffer);
  }

  static const uint32 VERSION = 2;
  static const uint32 POINTERS[];

  void SwapEndian() {
    FByteswapper(numTracks);
    FByteswapper(numFrames);
    FByteswapper(loopFrame);
    FByteswapper(endFrameAdditiveScenePosition);
    FByteswapper(endFrameAdditiveSceneRotation);
    FByteswapper(flags);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    tracks.Fixup(masterBuffer, swapEndian);
    eventTable.Fixup(masterBuffer, swapEndian);
    floatTracks.Fixup(masterBuffer, swapEndian);

    typename Traits::EventClass *dEvents = Events(masterBuffer);

    if (dEvents)
      for (uint32 e = 0; e < Traits::NUMEVENTGROUPS; e++)
        dEvents[e].Fixup(masterBuffer, swapEndian);

    if (swapEndian)
      SwapEndian();
  }
};

/************************************************************************/
/***************************** ANIM_V3 **********************************/
/************************************************************************/

template <class AnimTraits> struct AnimV3 {
  DECLARE_REFLECTOR;
  typedef AnimTraits Traits;

  typename Traits::Track_Pointer tracks;
  uint32 numTracks, numFrames;
  int32 loopFrame;
  Vector4A16 endFrameAdditiveScenePosition;
  Vector4A16 endFrameAdditiveSceneRotation;
  uint32 flags;
  typename Traits::Track_Pointer nullPtr[2];
  typename Traits::EventClass_Pointer eventTable;

  typename Traits::EventClass *Events(char *masterBuffer) {
    return eventTable.GetData(masterBuffer);
  }

  char *FloatTracks(char *) { return nullptr; }

  static const uint32 VERSION = 3;
  static const uint32 POINTERS[];

  void SwapEndian() {
    FByteswapper(numTracks);
    FByteswapper(numFrames);
    FByteswapper(loopFrame);
    FByteswapper(endFrameAdditiveScenePosition);
    FByteswapper(endFrameAdditiveSceneRotation);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    tracks.Fixup(masterBuffer, swapEndian);
    eventTable.Fixup(masterBuffer, swapEndian);

    if (swapEndian)
      SwapEndian();
  }
};

void LMTAnimation_internal::FrameRate(uint32 fps) {
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
public:
  typedef C value_type;
  typedef std::unique_ptr<C, es::deleter_hybrid> AnimationTypePtr;

  mutable AnimationTypePtr data;

  AnimShared() : data(new C) {}
  AnimShared(C *ptr, char *buff, bool swapEndian) : data(ptr, false) {
    if (!buff) {
      if (swapEndian)
        data->SwapEndian();

      return;
    }

    data->Fixup(buff, swapEndian);

    if (data->numTracks) {
      LMTConstructorProperties props;
      props.ptrSize =
          static_cast<uint8>(sizeof(typename C::Traits::Track_Pointer));
      props.version = C::Traits::TRACK_VERSION;
      props.masterBuffer = buff;
      props.swappedEndian = swapEndian;
      props.dataStart = data->tracks.GetData(buff);

      storage.emplace_back(LMTTrack::Create(props));

      const uint32 trackStride = storage[0]->Stride();

      char *&curData = reinterpret_cast<char *&>(props.dataStart);
      curData += trackStride;

      for (uint32 t = 1; t < data->numTracks; t++, curData += trackStride)
        storage.emplace_back(LMTTrack::Create(props));
    }

    LMTConstructorProperties eventProps;
    eventProps.ptrSize =
        static_cast<uint8>(sizeof(typename C::Traits::Track_Pointer));
    eventProps.swappedEndian = swapEndian;
    eventProps.dataStart = data->Events(buff);
    eventProps.masterBuffer = buff;
    eventProps.version = C::VERSION < 3 ? C::Traits::NUMEVENTGROUPS / 2 : 3;

    events = LMTEventsPtr(LMTAnimationEvent::Create(eventProps));

    eventProps.dataStart = data->FloatTracks(buff);
    eventProps.version = 0;

    if (eventProps.dataStart)
      floatTracks = LMTFloatTrackPtr(LMTFloatTrack::Create(eventProps));
  }

  void _ToXML(pugi::xml_node &node) const override {
    ReflectorWrapConst<const C> refl(data.get());
    ReflectorXMLUtil::Save(refl, node);
  }

  void _FromXML(pugi::xml_node &node) override {
    ReflectorWrap<C> refl(data.get());
    ReflectorXMLUtil::Load(refl, node);
  }

  std::vector<uint64> GetPtrValues() const override {
    std::vector<uint64> retVal;

    for (auto &p : data->POINTERS) {
      retVal.push_back(*reinterpret_cast<const uint64 *>(
          reinterpret_cast<const char *>(data.get()) + p));
    }

    return retVal;
  }

  LMTTrackPtr CreateTrack() const override {
    LMTConstructorProperties props;
    props.ptrSize = sizeof(typename C::Traits::Track_Pointer);
    props.version = C::Traits::TRACK_VERSION;

    return LMTTrackPtr(LMTTrack::Create(props));
  }

  LMTEventsPtr CreateEvents() const override {
    LMTConstructorProperties props;
    props.ptrSize = sizeof(typename C::Traits::Track_Pointer);
    props.version = C::VERSION < 3 ? C::Traits::NUMEVENTGROUPS / 2 : 3;

    return LMTEventsPtr(LMTAnimationEvent::Create(props));
  }

  LMTFloatTrackPtr CreateFloatTracks() const override {
    if (C::VERSION != 2)
      return nullptr;

    LMTConstructorProperties props;
    props.ptrSize = sizeof(typename C::Traits::Track_Pointer);
    props.version = 0;

    return LMTFloatTrackPtr(LMTFloatTrack::Create(props));
  }

  void _Save(BinWritterRef wr, LMTFixupStorage &storage) const override {
    const size_t cOff = wr.Tell();

    wr.Write<value_type &>(*data);

    for (auto &p : data->POINTERS)
      storage.SaveFrom(cOff + p);
  }

  uint32 GetVersion() const override { return C::VERSION; }

  bool _Is64bit() const override {
    return sizeof(typename C::Traits::Track_Pointer) == 8;
  }

  uint32 NumFrames() const override { return data->numFrames; }

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
        static_cast<const LMTTrack_internal &>(*storage[0]).useRefFrame;
  }
};

typedef AnimV0<AnimTraitsV1<PointerX86, 0>> AnimV0X86TrackV0;
static const uint32 AnimV0X86TrackV0_PTR_00 =
    offsetof(AnimV0X86TrackV0, tracks);
static const uint32 AnimV0X86TrackV0_PTR_01 =
    offsetof(AnimV0X86TrackV0, events[0].events);
template <>
const uint32 AnimV0X86TrackV0::POINTERS[] = {
    AnimV0X86TrackV0_PTR_00, AnimV0X86TrackV0_PTR_01,
    AnimV0X86TrackV0_PTR_01 + _EVENT_STRIDE};
REFLECTOR_CREATE(AnimV0X86TrackV0, 2, VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition);

typedef AnimV0<AnimTraitsV1<PointerX64, 0>> AnimV0X64TrackV0;
static const uint32 AnimV0X64TrackV0_PTR_00 =
    offsetof(AnimV0X64TrackV0, tracks);
static const uint32 AnimV0X64TrackV0_PTR_01 =
    offsetof(AnimV0X64TrackV0, events[0].events);
template <>
const uint32 AnimV0X64TrackV0::POINTERS[] = {
    AnimV0X64TrackV0_PTR_00, AnimV0X64TrackV0_PTR_01,
    AnimV0X64TrackV0_PTR_01 + _EVENT_STRIDE};
REFLECTOR_CREATE(AnimV0X64TrackV0, 2, VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition);

typedef AnimV1<AnimTraitsV1<PointerX86, 1>> AnimV1X86TrackV1;
static const uint32 AnimV1X86TrackV1_PTR_00 =
    offsetof(AnimV1X86TrackV1, tracks);
static const uint32 AnimV1X86TrackV1_PTR_01 =
    offsetof(AnimV1X86TrackV1, events[0].events);
template <>
const uint32 AnimV1X86TrackV1::POINTERS[] = {
    AnimV1X86TrackV1_PTR_00, AnimV1X86TrackV1_PTR_01,
    AnimV1X86TrackV1_PTR_01 + _EVENT_STRIDE};
REFLECTOR_CREATE(AnimV1X86TrackV1, 2, VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

typedef AnimV1<AnimTraitsV1<PointerX64, 1>> AnimV1X64TrackV1;
static const uint32 AnimV1X64TrackV1_PTR_00 =
    offsetof(AnimV1X64TrackV1, tracks);
static const uint32 AnimV1X64TrackV1_PTR_01 =
    offsetof(AnimV1X64TrackV1, events[0].events);
template <>
const uint32 AnimV1X64TrackV1::POINTERS[] = {
    AnimV1X64TrackV1_PTR_00, AnimV1X64TrackV1_PTR_01,
    AnimV1X64TrackV1_PTR_01 + _EVENT_STRIDE};
REFLECTOR_CREATE(AnimV1X64TrackV1, 2, VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

typedef AnimV1<AnimTraitsV1<PointerX86, 2>> AnimV1X86TrackV1_5;
static const uint32 AnimV1X86TrackV1_5_PTR_00 =
    offsetof(AnimV1X86TrackV1_5, tracks);
static const uint32 AnimV1X86TrackV1_5_PTR_01 =
    offsetof(AnimV1X86TrackV1_5, events[0].events);
template <>
const uint32 AnimV1X86TrackV1_5::POINTERS[] = {
    AnimV1X86TrackV1_5_PTR_00, AnimV1X86TrackV1_5_PTR_01,
    AnimV1X86TrackV1_5_PTR_01 + _EVENT_STRIDE};
REFLECTOR_CREATE(AnimV1X86TrackV1_5, 2, VARNAMES, TEMPLATE, numFrames,
                 loopFrame, endFrameAdditiveScenePosition,
                 endFrameAdditiveSceneRotation);

typedef AnimV1<AnimTraitsV1<PointerX64, 2>> AnimV1X64TrackV1_5;
static const uint32 AnimV1X64TrackV1_5_PTR_00 =
    offsetof(AnimV1X64TrackV1_5, tracks);
static const uint32 AnimV1X64TrackV1_5_PTR_01 =
    offsetof(AnimV1X64TrackV1_5, events[0].events);
template <>
const uint32 AnimV1X64TrackV1_5::POINTERS[] = {
    AnimV1X64TrackV1_5_PTR_00, AnimV1X64TrackV1_5_PTR_01,
    AnimV1X64TrackV1_5_PTR_01 + _EVENT_STRIDE};
REFLECTOR_CREATE(AnimV1X64TrackV1_5, 2, VARNAMES, TEMPLATE, numFrames,
                 loopFrame, endFrameAdditiveScenePosition,
                 endFrameAdditiveSceneRotation);

typedef AnimV1<AnimTraitsV2<PointerX86, 3>> AnimV1X86TrackV2;
static const uint32 AnimV1X86TrackV2_PTR_00 =
    offsetof(AnimV1X86TrackV2, tracks);
static const uint32 AnimV1X86TrackV2_PTR_01 =
    offsetof(AnimV1X86TrackV2, events[0].events);
template <>
const uint32 AnimV1X86TrackV2::POINTERS[] = {
    AnimV1X86TrackV2_PTR_00, AnimV1X86TrackV2_PTR_01,
    AnimV1X86TrackV2_PTR_01 + _EVENT_STRIDE,
    AnimV1X86TrackV2_PTR_01 + _EVENT_STRIDE * 2,
    AnimV1X86TrackV2_PTR_01 + _EVENT_STRIDE * 3};
REFLECTOR_CREATE(AnimV1X86TrackV2, 2, VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

typedef AnimV1<AnimTraitsV2<PointerX64, 3>> AnimV1X64TrackV2;
static const uint32 AnimV1X64TrackV2_PTR_00 =
    offsetof(AnimV1X64TrackV2, tracks);
static const uint32 AnimV1X64TrackV2_PTR_01 =
    offsetof(AnimV1X64TrackV2, events[0].events);
template <>
const uint32 AnimV1X64TrackV2::POINTERS[] = {
    AnimV1X64TrackV2_PTR_00, AnimV1X64TrackV2_PTR_01,
    AnimV1X64TrackV2_PTR_01 + _EVENT_STRIDE,
    AnimV1X64TrackV2_PTR_01 + _EVENT_STRIDE * 2,
    AnimV1X64TrackV2_PTR_01 + _EVENT_STRIDE * 3};
REFLECTOR_CREATE(AnimV1X64TrackV2, 2, VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

typedef AnimV2<AnimTraitsV2<PointerX86, 3>> AnimV2X86TrackV2;
static const uint32 AnimV2X86TrackV2_PTR_00 =
    offsetof(AnimV2X86TrackV2, tracks);
static const uint32 AnimV2X86TrackV2_PTR_01 =
    offsetof(AnimV2X86TrackV2, eventTable);
static const uint32 AnimV2X86TrackV2_PTR_02 =
    offsetof(AnimV2X86TrackV2, floatTracks);
template <>
const uint32 AnimV2X86TrackV2::POINTERS[] = {
    AnimV2X86TrackV2_PTR_00, AnimV2X86TrackV2_PTR_01, AnimV2X86TrackV2_PTR_02};
REFLECTOR_CREATE(AnimV2X86TrackV2, 2, VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

typedef AnimV2<AnimTraitsV2<PointerX64, 3>> AnimV2X64TrackV2;
static const uint32 AnimV2X64TrackV2_PTR_00 =
    offsetof(AnimV2X64TrackV2, tracks);
static const uint32 AnimV2X64TrackV2_PTR_01 =
    offsetof(AnimV2X64TrackV2, eventTable);
static const uint32 AnimV2X64TrackV2_PTR_02 =
    offsetof(AnimV2X64TrackV2, floatTracks);
template <>
const uint32 AnimV2X64TrackV2::POINTERS[] = {
    AnimV2X64TrackV2_PTR_00, AnimV2X64TrackV2_PTR_01, AnimV2X64TrackV2_PTR_02};
REFLECTOR_CREATE(AnimV2X64TrackV2, 2, VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

typedef AnimV3<AnimTraitsV2<PointerX86, 4>> AnimV3X86TrackV3;
static const uint32 AnimV3X86TrackV3_PTR_00 =
    offsetof(AnimV3X86TrackV3, tracks);
static const uint32 AnimV3X86TrackV3_PTR_01 =
    offsetof(AnimV3X86TrackV3, eventTable);
template <>
const uint32 AnimV3X86TrackV3::POINTERS[] = {AnimV3X86TrackV3_PTR_00,
                                             AnimV3X86TrackV3_PTR_01};
REFLECTOR_CREATE(AnimV3X86TrackV3, 2, VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

typedef AnimV3<AnimTraitsV2<PointerX64, 4>> AnimV3X64TrackV3;
static const uint32 AnimV3X64TrackV3_PTR_00 =
    offsetof(AnimV3X64TrackV3, tracks);
static const uint32 AnimV3X64TrackV3_PTR_01 =
    offsetof(AnimV3X64TrackV3, eventTable);
template <>
const uint32 AnimV3X64TrackV3::POINTERS[] = {AnimV3X64TrackV3_PTR_00,
                                             AnimV3X64TrackV3_PTR_01};
REFLECTOR_CREATE(AnimV3X64TrackV3, 2, VARNAMES, TEMPLATE, numFrames, loopFrame,
                 endFrameAdditiveScenePosition, endFrameAdditiveSceneRotation);

ES_STATIC_ASSERT(sizeof(AnimV1X86TrackV1) == 192);
ES_STATIC_ASSERT(sizeof(AnimV3X64TrackV3) == 96);
ES_STATIC_ASSERT(alignof(AnimV3X64TrackV3) == 16);
//ES_STATIC_ASSERT(offsetof(AnimV3X64TrackV3, eventTable) == 88);

template <class Derived> static LMTAnimation *_creattorBase() {
  return new AnimShared<Derived>();
}

template <class C>
static LMTAnimation *_creator(void *ptr, char *buff, bool endi) {
  return new AnimShared<C>(static_cast<C *>(ptr), buff, endi);
}

#define LMTANI_REG(version, ptrSize, ...)                                      \
  { 0x0##ptrSize | (LMT::V_##version << 8), _creattorBase<__VA_ARGS__> }

#define LMTANI_REG_LINK(version, ptrSize, ...)                                 \
  { 0x0##ptrSize | (LMT::V_##version << 8), _creator<__VA_ARGS__> }

static const std::unordered_map<uint16, LMTAnimation *(*)()> animationRegistry =
    {LMTANI_REG(22, 4, AnimV0X86TrackV0),
     LMTANI_REG(40, 4, AnimV1X86TrackV1),
     LMTANI_REG(49, 4, AnimV1X86TrackV1),
     LMTANI_REG(50, 4, AnimV1X86TrackV1),
     LMTANI_REG(51, 4, AnimV1X86TrackV1_5),
     LMTANI_REG(56, 4, AnimV1<AnimTraitsV2<PointerX86, 3>>),
     LMTANI_REG(57, 4, AnimV1<AnimTraitsV2<PointerX86, 3>>),
     LMTANI_REG(66, 4, AnimV2<AnimTraitsV2<PointerX86, 3>>),
     LMTANI_REG(67, 4, AnimV2<AnimTraitsV2<PointerX86, 3>>),
     LMTANI_REG(92, 4, AnimV3<AnimTraitsV2<PointerX86, 4>>),

     LMTANI_REG(22, 8, AnimV0X64TrackV0),
     LMTANI_REG(40, 8, AnimV1X64TrackV1),
     LMTANI_REG(49, 8, AnimV1X64TrackV1),
     LMTANI_REG(50, 8, AnimV1X64TrackV1),
     LMTANI_REG(51, 8, AnimV1X64TrackV1_5),
     LMTANI_REG(56, 8, AnimV1<AnimTraitsV2<PointerX64, 3>>),
     LMTANI_REG(57, 8, AnimV1<AnimTraitsV2<PointerX64, 3>>),
     LMTANI_REG(66, 8, AnimV2<AnimTraitsV2<PointerX64, 3>>),
     LMTANI_REG(67, 8, AnimV2<AnimTraitsV2<PointerX64, 3>>),
     LMTANI_REG(92, 8, AnimV3<AnimTraitsV2<PointerX64, 4>>)};

static const std::unordered_map<uint16, LMTAnimation *(*)(void *, char *, bool)>
    animationLinkRegistry = {
        LMTANI_REG_LINK(22, 4, AnimV0X86TrackV0),
        LMTANI_REG_LINK(40, 4, AnimV1X86TrackV1),
        LMTANI_REG_LINK(49, 4, AnimV1X86TrackV1),
        LMTANI_REG_LINK(50, 4, AnimV1X86TrackV1),
        LMTANI_REG_LINK(51, 4, AnimV1X86TrackV1_5),
        LMTANI_REG_LINK(56, 4, AnimV1<AnimTraitsV2<PointerX86, 3>>),
        LMTANI_REG_LINK(57, 4, AnimV1<AnimTraitsV2<PointerX86, 3>>),
        LMTANI_REG_LINK(66, 4, AnimV2<AnimTraitsV2<PointerX86, 3>>),
        LMTANI_REG_LINK(67, 4, AnimV2<AnimTraitsV2<PointerX86, 3>>),
        LMTANI_REG_LINK(92, 4, AnimV3<AnimTraitsV2<PointerX86, 4>>),

        LMTANI_REG_LINK(22, 8, AnimV0X64TrackV0),
        LMTANI_REG_LINK(40, 8, AnimV1X64TrackV1),
        LMTANI_REG_LINK(49, 8, AnimV1X64TrackV1),
        LMTANI_REG_LINK(50, 8, AnimV1X64TrackV1),
        LMTANI_REG_LINK(51, 8, AnimV1X64TrackV1_5),
        LMTANI_REG_LINK(56, 8, AnimV1<AnimTraitsV2<PointerX64, 3>>),
        LMTANI_REG_LINK(57, 8, AnimV1<AnimTraitsV2<PointerX64, 3>>),
        LMTANI_REG_LINK(66, 8, AnimV2<AnimTraitsV2<PointerX64, 3>>),
        LMTANI_REG_LINK(67, 8, AnimV2<AnimTraitsV2<PointerX64, 3>>),
        LMTANI_REG_LINK(92, 8, AnimV3<AnimTraitsV2<PointerX64, 4>>)};

static const std::list<LMT::V> supportedVersions = {
    LMT::V_22, LMT::V_40, LMT::V_49, LMT::V_50, LMT::V_51,
    LMT::V_56, LMT::V_57, LMT::V_66, LMT::V_67, LMT::V_92};

bool LMTAnimation::SupportedVersion(uint16 version) {
  for (auto &v : supportedVersions)
    if (v == version)
      return true;

  return false;
}

REGISTER_ENUMS(AnimV2Flags)

LMTAnimation *LMTAnimation::Create(const LMTConstructorProperties &props) {
  uint16 item = reinterpret_cast<const uint16 &>(props);

  RegisterLocalEnums();

  if (!animationRegistry.count(item))
    return nullptr;

  LMTAnimation *cAni = nullptr;

  if (props.dataStart)
    cAni = animationLinkRegistry.at(item)(props.dataStart, props.masterBuffer,
                                          props.swappedEndian);
  else
    cAni = animationRegistry.at(item)();

  cAni->props = props;

  return cAni;
}

bool IsX64CompatibleAnimationClass(BinReaderRef rd, uint16 version) {
  uint16 item = 0x8 | version << 8;

  RegisterLocalEnums();

  if (!animationRegistry.count(item))
    return false;

  char buffer[0x100];

  rd.ReadBuffer(buffer, 0x100);

  LMTAnimation_internal *cAni = static_cast<LMTAnimation_internal *>(
      animationLinkRegistry.at(item)(buffer, nullptr, rd.SwappedEndian()));

  auto ptrVals = cAni->GetPtrValues();

  for (auto v : ptrVals)
    if (v & (0xffffffffULL << 32))
      return false;

  return true;
}