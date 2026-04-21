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

#include "revil/srd.hpp"
#include "property.hpp"
#include "pugixml.hpp"
#include "spike/except.hpp"
#include "spike/io/binreader_stream.hpp"
#include <cassert>
#include <vector>

namespace {
struct Element {
  uint32 mReqNo;
  uint32 mRandomReqNo00;
  uint32 mPercent00;
  uint32 mRandomReqNo01;
  uint32 mPercent01;
  uint32 mRandomReqNo02;
  uint32 mPercent02;
  uint32 mRandomReqNo03;
  uint32 mPercent03;
  uint32 mRandomReqNo04;
  uint32 mPercent04;
  uint32 mRandomReqNo05;
  uint32 mPercent05;
  uint32 mRandomReqNo06;
  uint32 mPercent06;
  uint32 mRandomReqNo07;
  uint32 mPercent07;
  uint32 mRandomReqNo08;
  uint32 mPercent08;
  uint32 mRandomReqNo09;
  uint32 mPercent09;
  uint32 mRandomReqNo10;
  uint32 mPercent10;
  uint32 mRandomReqNo11;
  uint32 mPercent11;
  uint32 mRandomReqNo12;
  uint32 mPercent12;
  uint32 mRandomReqNo13;
  uint32 mPercent13;
  uint32 mRandomReqNo14;
  uint32 mPercent14;
  uint32 mRandomReqNo15;
  uint32 mPercent15;
  bool mIsSimpleRandomMode;
  int32 mLastReqNo;
};

struct SoundRandomHeader {
  uint32 id;
  revil::SRDVersion version;
  uint32 numElements;
  uint32 reserved;
};
} // namespace

void ToXML(const Element &data, pugi::xml_node node) {
  NewPrimitive(node, "mReqNo", data.mReqNo);
  NewPrimitive(node, "mRandomReqNo00", data.mRandomReqNo00);
  NewPrimitive(node, "mPercent00", data.mPercent00);
  NewPrimitive(node, "mRandomReqNo01", data.mRandomReqNo01);
  NewPrimitive(node, "mPercent01", data.mPercent01);
  NewPrimitive(node, "mRandomReqNo02", data.mRandomReqNo02);
  NewPrimitive(node, "mPercent02", data.mPercent02);
  NewPrimitive(node, "mRandomReqNo03", data.mRandomReqNo03);
  NewPrimitive(node, "mPercent03", data.mPercent03);
  NewPrimitive(node, "mRandomReqNo04", data.mRandomReqNo04);
  NewPrimitive(node, "mPercent04", data.mPercent04);
  NewPrimitive(node, "mRandomReqNo05", data.mRandomReqNo05);
  NewPrimitive(node, "mPercent05", data.mPercent05);
  NewPrimitive(node, "mRandomReqNo06", data.mRandomReqNo06);
  NewPrimitive(node, "mPercent06", data.mPercent06);
  NewPrimitive(node, "mRandomReqNo07", data.mRandomReqNo07);
  NewPrimitive(node, "mPercent07", data.mPercent07);
  NewPrimitive(node, "mRandomReqNo08", data.mRandomReqNo08);
  NewPrimitive(node, "mPercent08", data.mPercent08);
  NewPrimitive(node, "mRandomReqNo09", data.mRandomReqNo09);
  NewPrimitive(node, "mPercent09", data.mPercent09);
  NewPrimitive(node, "mRandomReqNo10", data.mRandomReqNo10);
  NewPrimitive(node, "mPercent10", data.mPercent10);
  NewPrimitive(node, "mRandomReqNo11", data.mRandomReqNo11);
  NewPrimitive(node, "mPercent11", data.mPercent11);
  NewPrimitive(node, "mRandomReqNo12", data.mRandomReqNo12);
  NewPrimitive(node, "mPercent12", data.mPercent12);
  NewPrimitive(node, "mRandomReqNo13", data.mRandomReqNo13);
  NewPrimitive(node, "mPercent13", data.mPercent13);
  NewPrimitive(node, "mRandomReqNo14", data.mRandomReqNo14);
  NewPrimitive(node, "mPercent14", data.mPercent14);
  NewPrimitive(node, "mRandomReqNo15", data.mRandomReqNo15);
  NewPrimitive(node, "mPercent15", data.mPercent15);
  NewPrimitive(node, "mIsSimpleRandomMode", data.mIsSimpleRandomMode);
  NewPrimitive(node, "mLastReqNo", data.mLastReqNo);
}

struct revil::SRDImpl {
  std::vector<Element> elements;

  void Load(BinReaderRef rd) {
    SoundRandomHeader hdr;
    rd.Read(hdr);

    if (hdr.id != CompileFourCC("DNRS")) {
      throw es::InvalidHeaderError(hdr.id);
    }

    if (hdr.version == SRDVersion::V_2) {
      rd.ReadContainer(elements, hdr.numElements);
    } else {
      throw es::InvalidVersionError(uint32(hdr.version));
    }
  }

  void ToXML(pugi::xml_node node) const {
    uint32 curId = 0;
    pugi::xml_node srdNode = NewProperty(MtPropertyType::class_, node);
    srdNode.append_attribute("type").set_value("rSoundRandom");
    srdNode.append_attribute("id").set_value(curId++);

    pugi::xml_node elementsNode =
        NewArray(MtPropertyType::class_, srdNode, "mpElement", elements.size());

    for (auto &s : elements) {
      pugi::xml_node curNode =
          NewProperty(MtPropertyType::class_, elementsNode);
      curNode.append_attribute("type").set_value("rSoundRandom::Element");
      curNode.append_attribute("id").set_value(curId++);
      ::ToXML(s, curNode);
    }
  }
};

namespace revil {
SRD::SRD() : pi(std::make_unique<SRDImpl>()) {}
SRD::~SRD() = default;

void SRD::Load(BinReaderRef_e rd) { pi->Load(rd); }

void SRD::ToXML(pugi::xml_node node) const { pi->ToXML(node); }
} // namespace revil
