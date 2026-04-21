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

#include "revil/cdf.hpp"
#include "property.hpp"
#include "pugixml.hpp"
#include "spike/except.hpp"
#include "spike/io/binreader_stream.hpp"
#include <vector>

struct ClothHeader {
  uint32 id;
  uint32 numChains;
  uint32 chainsOffset;
  uint32 reserved;
};

struct Constraint {
  uint8 index;
  uint8 type;
  uint8 targetJoint;
  uint8 axis;
  MtFloat3 minParam;
  MtFloat3 maxParam;
  float power;
};

struct ChainData0 {
  uint8 type;
  uint8 axis;
  bool applyWpos;
  uint8 numIndices;
  uint8 numConstraints;
  uint8 iterations;
};

struct ChainData1 {
  float blendRate;
  float gravity;
  float tailLength;
  uint32 indexOffset;
  uint32 constraintOffset;
};

struct Chain : ChainData0, ChainData1 {
  std::vector<Constraint> constraints;
  std::vector<uint32> joints;
};

void ToXML(const Constraint &data, pugi::xml_node node) {
  NewPrimitive(node, "index", data.index);
  NewPrimitive(node, "type", data.type);
  NewPrimitive(node, "targetJoint", data.targetJoint);
  NewPrimitive(node, "axis", data.axis);
  NewPrimitive(node, "minParam", data.minParam);
  NewPrimitive(node, "maxParam", data.maxParam);
  NewPrimitive(node, "power", data.power);
}

void ToXML(const Chain &data, pugi::xml_node node, uint32 &curId) {
  NewPrimitive(node, "type", data.type);
  NewPrimitive(node, "axis", data.axis);
  NewPrimitive(node, "applyWpos", data.applyWpos);
  NewPrimitive(node, "numIndices", data.numIndices);
  NewPrimitive(node, "numConstraints", data.numConstraints);
  NewPrimitive(node, "iterations", data.iterations);
  NewPrimitive(node, "blendRate", data.blendRate);
  NewPrimitive(node, "gravity", data.gravity);
  NewPrimitive(node, "tailLength", data.tailLength);

  pugi::xml_node jointsNode =
      NewArray(MtPropertyType::u32_, node, "jointIndices", data.joints.size());

  for (uint32 i : data.joints) {
    NewPrimitive(jointsNode, nullptr, i);
  }

  pugi::xml_node constraintsNode = NewArray(
      MtPropertyType::class_, node, "constraints", data.constraints.size());

  for (auto &c : data.constraints) {
    pugi::xml_node constraint =
        NewProperty(MtPropertyType::class_, constraintsNode);
    constraint.append_attribute("type").set_value("rCloth:Constraint");
    constraint.append_attribute("id").set_value(curId++);
    ToXML(c, constraint);
  }
}

struct revil::CDFImpl {
  std::vector<Chain> chains;

  void Load(BinReaderRef rd) {
    ClothHeader hdr;
    rd.Read(hdr);

    if (hdr.id != CompileFourCC("CDF\3")) {
      throw es::InvalidHeaderError(hdr.id);
    }

    chains.resize(hdr.numChains);

    for (uint32 i = 0; i < hdr.numChains; i++) {
      Chain &chain = chains.at(i);
      rd.ApplyPadding(16);
      rd.Read(static_cast<ChainData0 &>(chain));
      rd.Read(static_cast<ChainData1 &>(chain));
      rd.ApplyPadding(4);
      rd.ReadContainer(chain.joints, chain.numIndices);
      rd.ApplyPadding(16);
      rd.ReadContainer(chain.constraints, chain.numConstraints);
    }
  }

  void ToXML(pugi::xml_node node) const {
    uint32 curId = 0;
    pugi::xml_node clothNode = NewProperty(MtPropertyType::class_, node);
    clothNode.append_attribute("type").set_value("rCloth");
    clothNode.append_attribute("id").set_value(curId++);

    pugi::xml_node chainsNode =
        NewArray(MtPropertyType::classref, clothNode, "chains", chains.size());

    for (auto &c : chains) {
      pugi::xml_node chainNode =
          NewProperty(MtPropertyType::classref, chainsNode);
      chainNode.append_attribute("type").set_value("rCloth::Chain");
      chainNode.append_attribute("id").set_value(curId++);
      ::ToXML(c, chainNode, curId);
    }
  }
};

namespace revil {
CDF::CDF() : pi(std::make_unique<CDFImpl>()) {}
CDF::~CDF() = default;

void CDF::Load(BinReaderRef_e rd) { pi->Load(rd); }

void CDF::ToXML(pugi::xml_node node) const { pi->ToXML(node); }
} // namespace revil
