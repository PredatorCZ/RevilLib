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

#include "revil/sds.hpp"
#include "property.hpp"
#include "pugixml.hpp"
#include "spike/except.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/type/pointer.hpp"

namespace {
struct CurveElement {
  float angle;
  float value;
};

struct DirectionalCurve {
  uint32 numElements;
  float zeroRadianValue;
  float piRadianValue;
  es::PointerX86<CurveElement> elements;
};

struct SoundDirectionalSetList {
  int16 ID;
  es::PointerX86<DirectionalCurve> curve;
};

struct SoundDirectionalSetHeader {
  uint32 id;
  revil::SDSVersion version;
  uint32 numLists;
  uint32 numCurves;
  SoundDirectionalSetList list[];
};
} // namespace

void ToXML(const CurveElement &data, pugi::xml_node node) {
  NewPrimitive(node, "angle", data.angle);
  NewPrimitive(node, "value", data.value);
}

struct revil::SDSImpl {
  std::string buffer;
  SoundDirectionalSetHeader *hdr = nullptr;

  void Load(BinReaderRef rd) {
    uint32 id;
    rd.Push();
    rd.Read(id);
    rd.Pop();

    if (id != CompileFourCC("SDST")) {
      throw es::InvalidHeaderError(id);
    }

    rd.ReadContainer(buffer, rd.GetSize());
    hdr = reinterpret_cast<SoundDirectionalSetHeader *>(buffer.data());

    if (hdr->version != SDSVersion::V_3) {
      throw es::InvalidVersionError(int(hdr->version));
    }

    for (uint32 i = 0; i < hdr->numCurves; i++) {
      auto &cv = hdr->list[i].curve;
      cv.Fixup(buffer.data());
      cv->elements.Fixup(buffer.data());
    }
  }

  void ToXML(pugi::xml_node node) const {
    uint32 curId = 0;
    pugi::xml_node sdsNode = NewProperty(MtPropertyType::class_, node);
    sdsNode.append_attribute("type").set_value("rSoundDirectionalSetXml");
    sdsNode.append_attribute("id").set_value(curId++);
    pugi::xml_node listsNode =
        NewArray(MtPropertyType::class_, sdsNode, "mCurves", hdr->numLists);

    for (uint32 i = 0; i < hdr->numLists; i++) {
      auto &list = hdr->list[i];

      pugi::xml_node listNode = NewProperty(MtPropertyType::class_, listsNode);
      listNode.append_attribute("type").set_value(
          "rSoundDirectionalSetXml::Curve");
      listNode.append_attribute("id").set_value(curId++);
      NewPrimitive(listNode, "mID", list.ID);

      if (list.curve) {
        auto &nCurve = *list.curve;
        NewPrimitive(listNode, "mPiRadianValue", nCurve.piRadianValue);
        NewPrimitive(listNode, "mZeroRadianValue", nCurve.zeroRadianValue);
        pugi::xml_node curve = NewArray(MtPropertyType::class_, listNode,
                                        "mElements", nCurve.numElements);

        for (uint32 e = 0; e < nCurve.numElements; e++) {
          pugi::xml_node element = NewProperty(MtPropertyType::class_, curve);
          element.append_attribute("type").set_value(
              "rSoundDirectionalSetXml::Element");
          element.append_attribute("id").set_value(curId++);
          ::ToXML(nCurve.elements[e], element);
        }
      }
    }
  }
};

namespace revil {
SDS::SDS() : pi(std::make_unique<SDSImpl>()) {}
SDS::~SDS() = default;

void SDS::Load(BinReaderRef_e rd) { pi->Load(rd); }

void SDS::ToXML(pugi::xml_node node) const { pi->ToXML(node); }
} // namespace revil
