/*  Revil Format Library
    Copyright(C) 2017-2026 Lukas Cone

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

#include "material.hpp"
#include "../property.hpp"
#include <charconv>

// clang-format off
namespace {
void ToXML(const MODMaterialX70::VSHData &item, pugi::xml_node node) {
  NewPrimitive(node, "lightningType", item.Get<MODMaterialX70::LightningType>());
  NewPrimitive(node, "normalMapType", item.Get<MODMaterialX70::NormalMapType>());
  NewPrimitive(node, "specularType", item.Get<MODMaterialX70::SpecularType>());
  NewPrimitive(node, "lightMapType", item.Get<MODMaterialX70::LightMapType>());
  NewPrimitive(node, "multiTextureType", item.Get<MODMaterialX70::MultiTextureType>());
}

void ToXML(const MODMaterialX70::PSHData &item, pugi::xml_node node) {
  NewPrimitive(node, "enableFOG", bool(item.Get<MODMaterialX70::EnableFOG>()));
  NewPrimitive(node, "ZWrite", bool(item.Get<MODMaterialX70::ZWrite>()));
  NewPrimitive(node, "attr", item.Get<MODMaterialX70::Attr>());
  NewPrimitive(node, "no", item.Get<MODMaterialX70::No>());
  NewPrimitive(node, "envmapBias", item.Get<MODMaterialX70::EnvmapBias>());
  NewPrimitive(node, "VType", item.Get<MODMaterialX70::VType>());
  NewPrimitive(node, "enableUVScroll", bool(item.Get<MODMaterialX70::EnableUVScroll>()));
  NewPrimitive(node, "ZTest", bool(item.Get<MODMaterialX70::ZTest>()));
}

template<class Mat>
void ToXML(const Mat &item, pugi::xml_node node) {
  ToXML(item.vshData, node);
  ToXML(item.pshData, node);
  NewPrimitive(node, "technique", item.technique);
  NewPrimitive(node, "baseTextureIndex", item.baseTextureIndex);
  NewPrimitive(node, "normalTextureIndex", item.normalTextureIndex);
  NewPrimitive(node, "maskTextureIndex", item.maskTextureIndex);
  NewPrimitive(node, "lightTextureIndex", item.lightTextureIndex);
  NewPrimitive(node, "shadowTextureIndex", item.shadowTextureIndex);
  NewPrimitive(node, "additionalTextureIndex", item.additionalTextureIndex);
  NewPrimitive(node, "cubeMapTextureIndex", item.cubeMapTextureIndex);
  NewPrimitive(node, "heightTextureIndex", item.heightTextureIndex);
  NewPrimitive(node, "glossTextureIndex", item.glossTextureIndex);
  NewPrimitive(node, "transparency", item.transparency);
  NewPrimitive(node, "fresnelFactor", item.fresnelFactor);
  NewPrimitive(node, "lightMapScale", item.lightMapScale);
  NewPrimitive(node, "detailFactor", item.detailFactor);
  NewPrimitive(node, "transmit", item.transmit);
  NewPrimitive(node, "paralax", item.paralax);
  NewPrimitive(node, "blendState", item.blendState);
  NewPrimitive(node, "alphaRef", item.alphaRef);
}
}
// clang-format on

void MODMaterialX70::ToXML(pugi::xml_node node) const { ::ToXML(*this, node); }

void MODMaterialX170::ToXML(pugi::xml_node node) const { ::ToXML(*this, node); }

void MODMaterialXC5::ToXML(pugi::xml_node node) const { ::ToXML(*this, node); }

std::string MODMaterialX70::Name() const {
  char buffer[0x10]{};
  std::to_chars(std::begin(buffer), std::end(buffer), blendState, 0x10);
  return std::string("Material_") + buffer;
}

std::string MODMaterialHash::Name() const {
  char buffer[0x10]{};
  std::to_chars(std::begin(buffer), std::end(buffer), hash, 0x10);
  return std::string("Material_") + buffer;
}

void MODMaterialHash::ToXML(pugi::xml_node node) const {
  NewPrimitive(node, "hash", hash);
}

std::string MODMaterialName::Name() const { return name; }

void MODMaterialName::ToXML(pugi::xml_node node) const {
  pugi::xml_node matNode = NewProperty(MtPropertyType::string_, node);
  matNode.append_attribute("name").set_value("name");
  matNode.append_attribute("value").set_value(name);
}

void MODMaterialX21::ToXML(pugi::xml_node node) const {
  pugi::xml_node matNode = NewProperty(MtPropertyType::string_, node);
  matNode.append_attribute("name").set_value("name");
  matNode.append_attribute("value").set_value(name.c_str());
}
