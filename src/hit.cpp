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

#include "revil/hit.hpp"
#include "pugixml.hpp"
#include "shape_node.hpp"
#include "spike/except.hpp"
#include "spike/io/binreader_stream.hpp"
#include <vector>

struct HitNodeV1 : ShapeNodeV1 {
  int8 mType;
};

struct HitNodeV2 : ShapeNodeV2 {
  int8 mType;
};

struct HitHeader {
  uint32 id;
  revil::HITVersion version;
  uint32 numNodes;
  bool mDmgInfo;
};

void ToXML(const HitNodeV1 &data, pugi::xml_node node) {
  ToXML(static_cast<const ShapeNodeV1 &>(data), node);
  NewPrimitive(node, "mType", data.mType);
}

void ToXML(const HitNodeV2 &data, pugi::xml_node node) {
  ToXML(static_cast<const ShapeNodeV2 &>(data), node);
  NewPrimitive(node, "mType", data.mType);
}

struct revil::HITImpl {
  bool mDmgInfo;
  std::variant<std::vector<HitNodeV1>, std::vector<HitNodeV2>> nodes;

  size_t NumNodes() const {
    return std::visit([](auto &c) { return c.size(); }, nodes);
  }

  void Load(BinReaderRef rd) {
    HitHeader hdr;
    rd.Read(hdr);

    if (hdr.id != CompileFourCC("HIT ")) {
      throw es::InvalidHeaderError(hdr.id);
    }

    if (hdr.version == HITVersion::V_1) {
      mDmgInfo = hdr.mDmgInfo;
      auto &mNodes = nodes.emplace<std::vector<HitNodeV1>>();
      rd.ReadContainer(mNodes, hdr.numNodes);
    } else if (hdr.version == HITVersion::V_2) {
      mDmgInfo = hdr.mDmgInfo;
      auto &mNodes = nodes.emplace<std::vector<HitNodeV2>>();
      rd.ReadContainer(mNodes, hdr.numNodes);
    } else {
      throw es::InvalidVersionError(uint32(hdr.version));
    }
  }

  void ToXML(pugi::xml_node node) const {
    uint32 curId = 0;
    pugi::xml_node hitNode = NewProperty(MtPropertyType::class_, node);
    hitNode.append_attribute("type").set_value("rHitXml");
    hitNode.append_attribute("id").set_value(curId++);
    NewPrimitive(hitNode, "mDmgInfo", mDmgInfo);

    std::visit(
        [hitNode, &curId](auto &nodes) {
          pugi::xml_node nodesNode = NewArray(MtPropertyType::classref, hitNode,
                                              "mppNode", nodes.size());

          for (auto &s : nodes) {
            pugi::xml_node curNode =
                NewProperty(MtPropertyType::classref, nodesNode);
            curNode.append_attribute("type").set_value("rHit::Node");
            curNode.append_attribute("id").set_value(curId++);
            ::ToXML(s, curNode);
          }
        },
        nodes);
  }
};

namespace revil {
HIT::HIT() : pi(std::make_unique<HITImpl>()) {}
HIT::~HIT() = default;

void HIT::Load(BinReaderRef_e rd) { pi->Load(rd); }

void HIT::ToXML(pugi::xml_node node) const { pi->ToXML(node); }
} // namespace revil
