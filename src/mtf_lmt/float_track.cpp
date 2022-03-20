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
#include "datas/reflector_xml.hpp"
#include "fixup_storage.hpp"
#include "pugixml.hpp"

#include <array>
#include <map>

MAKE_ENUM(ENUMSCOPE(class FloatTrackComponentRemap
                    : uint8, FloatTrackComponentRemap),
          EMEMBER(NONE), EMEMBER(X_COMP), EMEMBER(Y_COMP), EMEMBER(Z_COMP));

template <template <class C> class PtrType> struct FloatTrack {
  FloatTrackComponentRemap componentRemaps[4];
  uint32 numFloats;
  PtrType<FloatFrame> frames;

  void SwapEndian() {
    FByteswapper(frames);
    FByteswapper(numFloats);
  }

  void Fixup(char *masterBuffer, bool swapEndian) {
    auto cb = [&] {
      if (swapEndian) {
        SwapEndian();
      }
    };

    if (!es::FixupPointersCB(masterBuffer, ptrStore, cb, frames)) {
      return;
    }
  }
};

REFLECT(CLASS(FloatTrack<esPointerX86>), MEMBER(componentRemaps));
REFLECT(CLASS(FloatTrack<esPointerX64>), MEMBER(componentRemaps));

template <class C> class FloatTracks_shared : public LMTFloatTrack_internal {
public:
  using GroupArray = std::array<C, 4>;

private:
  uni::Element<GroupArray> groups;

public:
  FloatTracks_shared() : groups(new GroupArray) {}
  FloatTracks_shared(C *fromPtr, char *masterBuffer, bool swapEndian) {
    groups = {reinterpret_cast<GroupArray *>(fromPtr), false};

    GroupArray &groupArray = *groups.get();
    size_t currentGroup = 0;

    for (auto &g : groupArray) {
      g.Fixup(masterBuffer, swapEndian);
      FloatFrame *groupFrames = g.frames;

      if (!groupFrames) {
        continue;
      }

      es::allocator_hybrid_base::LinkStorage(frames[currentGroup++],
                                             groupFrames, g.numFloats);
    }
  }

  void ReflectToXML(pugi::xml_node node, size_t groupID) const override {
    ReflectorWrap<const C> reflEvent((*groups)[groupID]);
    ReflectorXMLUtil::Save(reflEvent, node);
  }

  void ReflectFromXML(pugi::xml_node node, size_t groupID) override {
    ReflectorWrap<C> reflEvent(&(*groups)[groupID]);
    ReflectorXMLUtil::Load(reflEvent, node);
  }

  void SaveInternal(BinWritterRef wr, LMTFixupStorage &storage) const override {
    const size_t cOff = wr.Tell();
    size_t curGroup = 0;

    for (auto &g : *groups) {
      wr.Write(g);
      storage.SaveFrom(cOff + offsetof(C, frames) + sizeof(C) * curGroup++);
    }
  }

  bool Is64bit() const override {
    return sizeof(std::declval<C>().frames) == 8;
  }
};

using ptr_type_ = std::unique_ptr<LMTFloatTrack>;

template <class C> struct f_ {
  static ptr_type_ creatorBase() {
    return std::make_unique<FloatTracks_shared<C>>();
  }

  static ptr_type_ creator(void *ptr, char *buff, bool endi) {
    return std::make_unique<FloatTracks_shared<C>>(static_cast<C *>(ptr), buff,
                                                   endi);
  }
};

static const std::map<LMTConstructorPropertiesBase,
                      decltype(&f_<void>::creatorBase)>
    floatRegistry = {
        {{LMTArchType::X64, LMTVersion::Auto},
         f_<FloatTrack<esPointerX64>>::creatorBase},
        {{LMTArchType::X86, LMTVersion::Auto},
         f_<FloatTrack<esPointerX86>>::creatorBase},
};

static const std::map<LMTConstructorPropertiesBase,
                      decltype(&f_<void>::creator)>
    floatRegistryLink = {
        {{LMTArchType::X64, LMTVersion::Auto},
         f_<FloatTrack<esPointerX64>>::creator},
        {{LMTArchType::X86, LMTVersion::Auto},
         f_<FloatTrack<esPointerX86>>::creator},
};

ptr_type_ LMTFloatTrack::Create(const LMTConstructorProperties &props) {
  if (props.dataStart) {
    return floatRegistryLink.at(props)(props.dataStart, props.masterBuffer,
                                       props.swappedEndian);
  } else {
    return floatRegistry.at(props)();
  }
}
