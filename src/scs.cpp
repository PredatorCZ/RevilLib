/*  Revil Format Library
    Copyright(C) 2026 Lukas Cone

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

#include "revil/scs.hpp"
#include "pugixml.hpp"
#include "property.hpp"
#include "spike/except.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/type/pointer.hpp"
#include <span>

struct SoundCurveSetList {
  int16 ID;
  int16 volumeCurve;
  int16 reverbSendCurve;
  int16 LFECurve;
};

struct CurveElement {
  float distance;
  float value;
};

struct Curve {
  uint32 numElements;
  float maxDistnace;
  float minValue;
  float maxValue;
  CurveElement elements[];
};

struct SoundCurveSetHeader {
  uint32 id;
  revil::SCSVersion vesrion;
  uint32 numLists;
  uint32 numCurves;
  SoundCurveSetList list[];

  std::span<es::PointerX86<Curve>> Curves() {
    return {reinterpret_cast<es::PointerX86<Curve> *>(list + numLists),
            numCurves};
  }
};

void ToXML(const CurveElement &data, pugi::xml_node node) {
  NewPrimitive(node, "distance", data.distance);
  NewPrimitive(node, "value", data.value);
}

struct revil::SCSImpl {
  std::string buffer;
  SoundCurveSetHeader *hdr = nullptr;

  void Load(BinReaderRef rd) {
    uint32 id;
    rd.Push();
    rd.Read(id);
    rd.Pop();

    if (id != CompileFourCC("SCST")) {
      throw es::InvalidHeaderError(id);
    }

    rd.ReadContainer(buffer, rd.GetSize());
    hdr = reinterpret_cast<SoundCurveSetHeader *>(buffer.data());

    if (hdr->vesrion != SCSVersion::V_3) {
      throw es::InvalidVersionError(int(hdr->vesrion));
    }

    for (auto &p : hdr->Curves()) {
      p.Fixup(buffer.data());
    }
  }

  void ToXML(pugi::xml_node node) const {
    uint32 curId = 0;
    pugi::xml_node scsNode = NewProperty(MtPropertyType::class_, node);
    scsNode.append_attribute("type").set_value("rSoundCurveSetXml");
    scsNode.append_attribute("id").set_value(curId++);
    pugi::xml_node listsNode =
        NewArray(MtPropertyType::class_, scsNode, "mCurveLists", hdr->numLists);

    for (uint32 i = 0; i < hdr->numLists; i++) {
      auto &list = hdr->list[i];

      pugi::xml_node listNode = NewProperty(MtPropertyType::class_, listsNode);
      listNode.append_attribute("type").set_value(
          "rSoundCurveSetXml::BasicCurves");
      listNode.append_attribute("id").set_value(curId++);
      NewPrimitive(listNode, "mID", list.ID);

      if (list.volumeCurve > -1) {
        auto &nCurve = *hdr->Curves()[list.volumeCurve];
        pugi::xml_node curve = NewArray(MtPropertyType::class_, listNode,
                                        "mpVolumeCurve", nCurve.numElements);

        for (uint32 e = 0; e < nCurve.numElements; e++) {
          pugi::xml_node element =
              NewProperty(MtPropertyType::class_, curve);
          element.append_attribute("type").set_value(
              "rSoundCurveSetXml::Element");
          element.append_attribute("id").set_value(curId++);
          ::ToXML(nCurve.elements[e], element);
        }
      }

      if (list.reverbSendCurve > -1) {
        auto &nCurve = *hdr->Curves()[list.reverbSendCurve];
        pugi::xml_node curve = NewArray(MtPropertyType::class_, listNode,
                                        "mpReverbSendCurve", nCurve.numElements);

        for (uint32 e = 0; e < nCurve.numElements; e++) {
          pugi::xml_node element =
              NewProperty(MtPropertyType::class_, curve);
          element.append_attribute("type").set_value(
              "rSoundCurveSetXml::Element");
          element.append_attribute("id").set_value(curId++);
          ::ToXML(nCurve.elements[e], element);
        }
      }

      if (list.LFECurve > -1) {
        auto &nCurve = *hdr->Curves()[list.LFECurve];
        pugi::xml_node curve = NewArray(MtPropertyType::class_, listNode,
                                        "mpLFECurve", nCurve.numElements);

        for (uint32 e = 0; e < nCurve.numElements; e++) {
          pugi::xml_node element =
              NewProperty(MtPropertyType::class_, curve);
          element.append_attribute("type").set_value(
              "rSoundCurveSetXml::Element");
          element.append_attribute("id").set_value(curId++);
          ::ToXML(nCurve.elements[e], element);
        }
      }
    }
  }
};

namespace revil {
SCS::SCS() : pi(std::make_unique<SCSImpl>()) {}
SCS::~SCS() = default;

void SCS::Load(BinReaderRef_e rd) { pi->Load(rd); }

void SCS::ToXML(pugi::xml_node node) const { pi->ToXML(node); }
} // namespace revil
