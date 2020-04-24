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

#include "float_track.hpp"
#include "fixup_storage.hpp"
#include "datas/reflector_xml.hpp"

#include <array>
#include <unordered_map>

REFLECTOR_CREATE(FloatTrackComponentRemap, ENUM, 2, CLASS, 8, NONE, X_COMP,
                 Y_COMP, Z_COMP);

template <template <class C> class PtrType> struct FloatTrack {
  DECLARE_REFLECTOR;

  FloatTrackComponentRemap componentRemaps[4];
  uint32 numFloats;
  PtrType<FloatFrame> frames;

  void SwapEndian() { FByteswapper(numFloats); }

  void Fixup(char *masterBuffer, bool swapEndian) {
    frames.Fixup(masterBuffer, swapEndian);

    if (swapEndian)
      SwapEndian();
  }
};

typedef FloatTrack<PointerX86> FloatEventGroupPointerX86;
REFLECTOR_CREATE(FloatEventGroupPointerX86, 2, VARNAMES, TEMPLATE,
                 componentRemaps);

typedef FloatTrack<PointerX64> FloatEventGroupPointerX64;
REFLECTOR_CREATE(FloatEventGroupPointerX64, 2, VARNAMES, TEMPLATE,
                 componentRemaps);

template <class C> class FloatTracks_shared : public LMTFloatTrack_internal {
public:
  typedef std::array<C, 4> GroupArray;
  typedef std::unique_ptr<GroupArray, es::deleter_hybrid> GroupsPtr;

private:
  GroupsPtr groups;

public:
  FloatTracks_shared() : groups(new typename GroupsPtr::element_type()) {}
  FloatTracks_shared(C *fromPtr, char *masterBuffer, bool swapEndian) {
    groups = GroupsPtr(reinterpret_cast<GroupArray *>(fromPtr), false);

    GroupArray &groupArray = *groups.get();
    uint32 currentGroup = 0;

    for (auto &g : groupArray) {
      g.Fixup(masterBuffer, swapEndian);
      FloatFrame *groupFrames = g.frames.GetData(masterBuffer);

      if (!groupFrames)
        continue;

      frames[currentGroup++] =
          FramesCollection(groupFrames, groupFrames + g.numFloats,
                           FramesCollection::allocator_type(groupFrames));
    }
  }

  void _ToXML(pugi::xml_node &node, uint32 groupID) const override {
    ReflectorWrapConst<const C> reflEvent(&(*groups)[groupID]);
    ReflectorXMLUtil::Save(reflEvent, node);
  }

  void _FromXML(pugi::xml_node &node, uint32 groupID) override {
    ReflectorWrap<C> reflEvent(&(*groups)[groupID]);
    ReflectorXMLUtil::Load(reflEvent, node);
  }

  void _Save(BinWritterRef wr, LMTFixupStorage &storage) const override {
    const size_t cOff = wr.Tell();
    uint32 curGroup = 0;

    for (auto &g : *groups) {
      wr.Write(g);
      storage.SaveFrom(cOff + offsetof(C, frames) + sizeof(C) * curGroup++);
    }
  }

  bool _Is64bit() const override { return sizeof(C::frames) == 8; }
};

template <class C> static LMTFloatTrack *_creattorBase() {
  return new FloatTracks_shared<C>;
}

template <class C>
static LMTFloatTrack *_creator(void *ptr, char *buff, bool endi) {
  return new FloatTracks_shared<C>(static_cast<C *>(ptr), buff, endi);
}

static const std::unordered_map<uint16, LMTFloatTrack *(*)()> floatRegistry = {
    {0x8, _creattorBase<FloatEventGroupPointerX64>},
    {0x4, _creattorBase<FloatEventGroupPointerX86>}};

static const std::unordered_map<uint16, LMTFloatTrack *(*)(void *, char *, bool)>
    floatRegistryLink = {{0x8, _creator<FloatEventGroupPointerX64>},
                         {0x4, _creator<FloatEventGroupPointerX86>}};

REGISTER_ENUMS(FloatTrackComponentRemap)

LMTFloatTrack *LMTFloatTrack::Create(const LMTConstructorProperties &props) {
  uint16 item = reinterpret_cast<const uint16 &>(props);

  RegisterLocalEnums();

  if (!floatRegistry.count(item))
    return nullptr;

  if (props.dataStart)
    return floatRegistryLink.at(item)(props.dataStart, props.masterBuffer,
                                      props.swappedEndian);
  else
    return floatRegistry.at(item)();
}
