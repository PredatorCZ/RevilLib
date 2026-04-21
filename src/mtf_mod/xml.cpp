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

#include "../property.hpp"
#include "revil/mod.hpp"

namespace revil {
void MOD::ToXML(pugi::xml_node node) const {
  uint32 curId = 0;
  pugi::xml_node modNode = NewProperty(MtPropertyType::class_, node);
  modNode.append_attribute("type").set_value("rModelInfo");
  modNode.append_attribute("id").set_value(curId++);

  pugi::xml_node clustersNode = NewArray(MtPropertyType::class_, modNode,
                                         "mpCluster", Primitives().size());

  for (auto &p : Primitives()) {
    char name[0x20];
    snprintf(name, sizeof(name), "Mesh[%u:%u]", p.meshId, p.groupId);

    pugi::xml_node clusterNode =
        NewProperty(MtPropertyType::class_, clustersNode);
    clusterNode.append_attribute("type").set_value("rModeInfo::Cluster");
    clusterNode.append_attribute("id").set_value(curId++);
    pugi::xml_node cName = NewProperty(MtPropertyType::string_, clusterNode);
    cName.append_attribute("name").set_value("name");
    cName.append_attribute("value").set_value(name);
    NewPrimitive(clusterNode, "visible", p.flags[MODPrimitive::Flags::Visible]);
    NewPrimitive(clusterNode, "shape", p.flags[MODPrimitive::Flags::Shape]);
    NewPrimitive(clusterNode, "bridge", p.flags[MODPrimitive::Flags::Bridge]);
    NewPrimitive(clusterNode, "connective",
                 p.flags[MODPrimitive::Flags::Connective]);
    NewPrimitive(clusterNode, "sort", p.flags[MODPrimitive::Flags::Sort]);
    NewPrimitive(clusterNode, "binormalFlip",
                 p.flags[MODPrimitive::Flags::BinormalFlip]);
    NewPrimitive(clusterNode, "materialIndex", p.materialIndex);
    NewPrimitive(clusterNode, "drawMode", p.drawMode);
    NewPrimitive(clusterNode, "alphaType", p.alphaType);
  }

  auto textures = Textures();
  pugi::xml_node texturesNode =
      NewArray(MtPropertyType::custom, modNode, "mppTextures", textures.size());
  for (auto &t : textures) {
    pugi::xml_node textureNode =
        NewProperty(MtPropertyType::custom, texturesNode);
    textureNode.append_attribute("ctype").set_value("resource");
    textureNode.append_attribute("rtype").set_value("rTexture");
    textureNode.append_attribute("path").set_value(t.c_str());
  }

  pugi::xml_node materialsNode = NewArray(MtPropertyType::class_, modNode,
                                          "mpMaterials", Materials().size());

  for (auto &m : Materials()) {
    pugi::xml_node materialNode =
        NewProperty(MtPropertyType::class_, materialsNode);
    materialNode.append_attribute("type").set_value("rModeInfo::Material");
    materialNode.append_attribute("id").set_value(curId++);
    m->ToXML(materialNode);
  }

  const MODMetaData &metadata = Metadata();
  NewPrimitive(modNode, "middleDistance", metadata.middleDistance);
  NewPrimitive(modNode, "lowDistance", metadata.lowDistance);
  NewPrimitive(modNode, "lightGroup", metadata.lightGroup);
  NewPrimitive(modNode, "boundaryJoint", metadata.boundaryJoint);
}
} // namespace revil
