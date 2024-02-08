#include "material.hpp"
#include "spike/reflect/reflector.hpp"
#include <charconv>

REFLECT(CLASS(MODMaterialX70::VSHData),
        BITMEMBERNAME(MODMaterialX70::Unk00, "unknown00"),
        BITMEMBERNAME(MODMaterialX70::LightningType, "lightningType"),
        BITMEMBERNAME(MODMaterialX70::NormalMapType, "normalMapType"),
        BITMEMBERNAME(MODMaterialX70::SpecularType, "specularType"),
        BITMEMBERNAME(MODMaterialX70::LightMapType, "lightMapType"),
        BITMEMBERNAME(MODMaterialX70::MultiTextureType, "multiTextureType"),
        BITMEMBERNAME(MODMaterialX70::Unk01, "unknown01"),
        BITMEMBERNAME(MODMaterialX70::Unk02, "unknown02"));

REFLECT(CLASS(MODMaterialX70), MEMBER(pshData), MEMBER(vshData),
        MEMBER(baseTextureIndex), MEMBER(normalTextureIndex),
        MEMBER(maskTextureIndex), MEMBER(lightTextureIndex),
        MEMBER(shadowTextureIndex), MEMBER(additionalTextureIndex),
        MEMBER(cubeMapTextureIndex), MEMBER(detailTextureIndex),
        MEMBER(AOTextureIndex), MEMBER(transparency), MEMBER(fresnelFactor),
        MEMBER(fresnelBias), MEMBER(specularPower), MEMBER(envMapPower),
        MEMBER(lightMapScale), MEMBER(detailFactor), MEMBER(detailWrap),
        MEMBER(envMapBias), MEMBER(normalBias), MEMBER(transmit),
        MEMBER(paralax), MEMBER(hash), MEMBER(unk));

REFLECT(CLASS(MODMaterialX170), MEMBER(pshData), MEMBER(vshData),
        MEMBER(baseTextureIndex), MEMBER(normalTextureIndex),
        MEMBER(maskTextureIndex), MEMBER(lightTextureIndex),
        MEMBER(shadowTextureIndex), MEMBER(additionalTextureIndex),
        MEMBER(cubeMapTextureIndex), MEMBER(detailTextureIndex),
        MEMBER(AOTextureIndex), MEMBER(transparency), MEMBER(unk00),
        MEMBER(fresnelFactor), MEMBER(fresnelBias), MEMBER(specularPower),
        MEMBER(envMapPower), MEMBER(lightMapScale), MEMBER(detailFactor),
        MEMBER(detailWrap), MEMBER(envMapBias), MEMBER(normalBias),
        MEMBER(transmit), MEMBER(paralax), MEMBER(hash), MEMBER(unk01));

REFLECT(CLASS(MODMaterialXC5::VSHData),
        BITMEMBERNAME(MODMaterialXC5::LightningType, "lightningType"),
        BITMEMBERNAME(MODMaterialXC5::NormalMapType, "normalMapType"),
        BITMEMBERNAME(MODMaterialXC5::SpecularType, "specularType"),
        BITMEMBERNAME(MODMaterialXC5::LightMapType, "lightMapType"),
        BITMEMBERNAME(MODMaterialXC5::MultiTextureType, "multiTextureType"));

REFLECT(CLASS(MODMaterialXC5::PSHData),
        BITMEMBERNAME(MODMaterialXC5::Unk02, "unknown00"),
        BITMEMBERNAME(MODMaterialXC5::Unk03, "unknown01"),
        BITMEMBERNAME(MODMaterialXC5::Unk04, "unknown02"),
        BITMEMBERNAME(MODMaterialXC5::Unk05, "unknown03"),
        BITMEMBERNAME(MODMaterialXC5::Unk06, "unknown04"),
        BITMEMBERNAME(MODMaterialXC5::Unk07, "unknown05"),
        BITMEMBERNAME(MODMaterialXC5::Unk08, "unknown06"),
        BITMEMBERNAME(MODMaterialXC5::Unk09, "unknown07"),
        BITMEMBERNAME(MODMaterialXC5::Unk10, "unknown08"),
        BITMEMBERNAME(MODMaterialXC5::Unk11, "unknown09"));

REFLECT(CLASS(MODMaterialXC5), MEMBER(pshData), MEMBER(vshData),
        MEMBER(baseTextureIndex), MEMBER(normalTextureIndex),
        MEMBER(maskTextureIndex), MEMBER(lightTextureIndex),
        MEMBER(shadowTextureIndex), MEMBER(additionalTextureIndex),
        MEMBER(cubeMapTextureIndex), MEMBER(detailTextureIndex),
        MEMBER(AOTextureIndex), MEMBER(transparency), MEMBER(unk01),
        MEMBER(specularPower), MEMBER(envMapPower), MEMBER(lightMapScale),
        MEMBER(detailFactor), MEMBER(detailWrap), MEMBER(envMapBias),
        MEMBER(normalBias), MEMBER(unk02), MEMBER(unk03), MEMBER(unk04),
        MEMBER(unk05), MEMBER(unk06), MEMBER(unk07), MEMBER(unk08));

REFLECT(CLASS(MODMaterialHash), MEMBER(hash));

REFLECT(CLASS(MODMaterialName), MEMBER(name));

REFLECT(CLASS(MODMaterialX21), MEMBER(name));

std::string MODMaterialX70::Name() const {
  char buffer[0x10]{};
  std::to_chars(std::begin(buffer), std::end(buffer), hash, 0x10);
  return std::string("Material_") + buffer;
}

std::string MODMaterialHash::Name() const {
  char buffer[0x10]{};
  std::to_chars(std::begin(buffer), std::end(buffer), hash, 0x10);
  return std::string("Material_") + buffer;
}

std::string MODMaterialName::Name() const { return name; }
