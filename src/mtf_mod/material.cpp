#include "material.hpp"
#include "datas/reflector.hpp"

REFLECTOR_CREATE(MODMaterialX70::VSHData, BITFIELD, 1, EXTENDED,
                 (A, MODMaterialX70::Unk00, "unknown00"),
                 (A, MODMaterialX70::LightningType, "lightningType"),
                 (A, MODMaterialX70::NormalMapType, "normalMapType"),
                 (A, MODMaterialX70::SpecularType, "specularType"),
                 (A, MODMaterialX70::LightMapType, "lightMapType"),
                 (A, MODMaterialX70::MultiTextureType, "multiTextureType"),
                 (A, MODMaterialX70::Unk01, "unknown01"),
                 (A, MODMaterialX70::Unk02, "unknown02"));

REFLECTOR_CREATE(MODMaterialX70, 1, VARNAMES, pshData, vshData,
                 baseTextureIndex, normalTextureIndex, maskTextureIndex,
                 lightTextureIndex, shadowTextureIndex, additionalTextureIndex,
                 cubeMapTextureIndex, detailTextureIndex, AOTextureIndex,
                 transparency, fresnelFactor, fresnelBias, specularPower,
                 envMapPower, lightMapScale, detailFactor, detailWrap,
                 envMapBias, normalBias, transmit, paralax, hash, unk);

REFLECTOR_CREATE(MODMaterialX170, 1, VARNAMES, pshData, vshData,
                 baseTextureIndex, normalTextureIndex, maskTextureIndex,
                 lightTextureIndex, shadowTextureIndex, additionalTextureIndex,
                 cubeMapTextureIndex, detailTextureIndex, AOTextureIndex,
                 transparency, unk00, fresnelFactor, fresnelBias, specularPower,
                 envMapPower, lightMapScale, detailFactor, detailWrap,
                 envMapBias, normalBias, transmit, paralax, hash, unk01);

REFLECTOR_CREATE(MODMaterialXC5::VSHData, BITFIELD, 1, EXTENDED,
                 (A, MODMaterialXC5::LightningType, "lightningType"),
                 (A, MODMaterialXC5::NormalMapType, "normalMapType"),
                 (A, MODMaterialXC5::SpecularType, "specularType"),
                 (A, MODMaterialXC5::LightMapType, "lightMapType"),
                 (A, MODMaterialXC5::MultiTextureType, "multiTextureType"));

REFLECTOR_CREATE(MODMaterialXC5::PSHData, BITFIELD, 1, EXTENDED,
                 (A, MODMaterialXC5::Unk02, "unknown00"),
                 (A, MODMaterialXC5::Unk03, "unknown01"),
                 (A, MODMaterialXC5::Unk04, "unknown02"),
                 (A, MODMaterialXC5::Unk05, "unknown03"),
                 (A, MODMaterialXC5::Unk06, "unknown04"),
                 (A, MODMaterialXC5::Unk07, "unknown05"),
                 (A, MODMaterialXC5::Unk08, "unknown06"),
                 (A, MODMaterialXC5::Unk09, "unknown07"),
                 (A, MODMaterialXC5::Unk10, "unknown08"),
                 (A, MODMaterialXC5::Unk11, "unknown09"));

REFLECTOR_CREATE(MODMaterialXC5, 1, VARNAMES, pshData, vshData,
                 baseTextureIndex, normalTextureIndex, maskTextureIndex,
                 lightTextureIndex, shadowTextureIndex, additionalTextureIndex,
                 cubeMapTextureIndex, detailTextureIndex, AOTextureIndex,
                 transparency, unk01, specularPower, envMapPower, lightMapScale,
                 detailFactor, detailWrap, envMapBias, normalBias, unk02, unk03,
                 unk04, unk05, unk06, unk07, unk08);

void RegisterMaterials() {
  RegisterReflectedTypes<MODMaterialX70::VSHData, MODMaterialXC5::VSHData,
                         MODMaterialXC5::PSHData>();
}
