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

#include "float_track.hpp"
#include "fixup_storage.hpp"
#include "pugixml.hpp"
#include "spike/reflect/reflector_xml.hpp"

#include "float_track.inl"

REFLECT(ENUMERATION(FloatTrackComponentRemap), ENUM_MEMBER(NONE),
        ENUM_MEMBER(X_COMP), ENUM_MEMBER(Y_COMP), ENUM_MEMBER(Z_COMP));

void FByteswapper(FloatFrame &item) {
  FByteswapper(item.data);
  FByteswapper(item.value);
}

class FloatTracksMidInterface : public LMTFloatTrack_internal {
  clgen::FloatTracks::Interface interface;

public:
  FloatTracksMidInterface(clgen::LayoutLookup rules, char *data) : interface {
    data, rules
  } {
  }

  void Fixup(char *root, bool swapEndian, std::vector<void *> &ptrStore) {
    for (auto g : interface.Groups()) {
      if (g.FramesPtr().Check(ptrStore)) {
        return;
      }

      if (swapEndian) {
        clgen::EndianSwap(interface);
      }

      g.FramesPtr().Fixup(root, ptrStore);

      if (swapEndian) {
        auto frames = g.Frames();
        for (size_t t = 0; t < g.NumFloats(); t++) {
          FByteswapper(frames[t]);
        }
      }
    }
  }

  bool Is64bit() const { return interface.lookup.x64; }

  size_t GetGroupTrackCount(size_t groupID) const override {
    return interface.Groups().at(groupID).NumFloats();
  }

  const FloatFrame *GetFrames(size_t groupID) const override {
    return interface.Groups().at(groupID).Frames();
  }

  FloatFrame *GetFrames(size_t groupID) override {
    return interface.Groups().at(groupID).Frames();
  }
};

using ptr_type_ = std::unique_ptr<LMTFloatTrack>;

ptr_type_ LMTFloatTrack::Create(const LMTConstructorProperties &props) {
  return std::make_unique<FloatTracksMidInterface>(
      clgen::LayoutLookup{static_cast<uint8>(props.version),
                          props.arch == LMTArchType::X64, false},
      static_cast<char *>(props.dataStart));
}
