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

#include "revil/oba.hpp"
#include "pugixml.hpp"
#include "shape_node.hpp"
#include "spike/except.hpp"
#include "spike/io/binreader_stream.hpp"
#include <vector>

struct ObjAdjNodeV1 : ShapeNodeV1 {
  int8 mType;
  float mWeight;
};

struct ObjAdjNodeV2 : ShapeNodeV2 {
  int8 mType;
  float mWeight;
};

struct ObjAdjHeader {
  uint32 id;
  revil::OBAVersion version;
  uint32 numNodes;
};

void ToXML(const ObjAdjNodeV1 &data, pugi::xml_node node) {
  ToXML(static_cast<const ShapeNodeV1 &>(data), node);
  NewPrimitive(node, "mType", data.mType);
  NewPrimitive(node, "mWeight", data.mWeight);
}

void ToXML(const ObjAdjNodeV2 &data, pugi::xml_node node) {
  ToXML(static_cast<const ShapeNodeV2 &>(data), node);
  NewPrimitive(node, "mType", data.mType);
  NewPrimitive(node, "mWeight", data.mWeight);
}

struct revil::OBAImpl {
  std::variant<std::vector<ObjAdjNodeV1>, std::vector<ObjAdjNodeV2>> nodes;

  size_t NumNodes() const {
    return std::visit([](auto &c) { return c.size(); }, nodes);
  }

  void Load(BinReaderRef rd) {
    ObjAdjHeader hdr;
    rd.Read(hdr);

    if (hdr.id != CompileFourCC("OBJA")) {
      throw es::InvalidHeaderError(hdr.id);
    }

    if (hdr.version == OBAVersion::V_1) {
      auto &mNodes = nodes.emplace<std::vector<ObjAdjNodeV1>>();
      rd.ReadContainer(mNodes, hdr.numNodes);
    } else if (hdr.version == OBAVersion::V_2) {
      auto &mNodes = nodes.emplace<std::vector<ObjAdjNodeV2>>();
      rd.ReadContainer(mNodes, hdr.numNodes);
    } else {
      throw es::InvalidVersionError(uint32(hdr.version));
    }
  }

  void ToXML(pugi::xml_node node) const {
    uint32 curId = 0;
    pugi::xml_node objNode = NewProperty(MtPropertyType::class_, node);
    objNode.append_attribute("type").set_value("rObjAdjXml");
    objNode.append_attribute("id").set_value(curId++);

    std::visit(
        [objNode, &curId](auto &nodes) {
          pugi::xml_node nodesNode = NewArray(MtPropertyType::classref, objNode,
                                              "mppNode", nodes.size());

          for (auto &s : nodes) {
            pugi::xml_node curNode =
                NewProperty(MtPropertyType::classref, nodesNode);
            curNode.append_attribute("type").set_value("rObjAdj::Node");
            curNode.append_attribute("id").set_value(curId++);
            ::ToXML(s, curNode);
          }
        },
        nodes);
  }
};

namespace revil {
OBA::OBA() : pi(std::make_unique<OBAImpl>()) {}
OBA::~OBA() = default;

void OBA::Load(BinReaderRef_e rd) { pi->Load(rd); }

void OBA::ToXML(pugi::xml_node node) const { pi->ToXML(node); }
} // namespace revil
