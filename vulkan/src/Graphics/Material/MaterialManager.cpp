#include "MaterialManager.h"
#include <memory>
#include "IMaterial.h"
#include "PBRMaterial.h"

MaterialManager::MaterialManager(Device& device, TextureManager& textureManager)
    : device(device), textureManager(textureManager)
{
}

MaterialManager::~MaterialManager()
{
}

MaterialID MaterialManager::LoadMaterialPBRgltf(MaterialManager::PBR_GLTF_LoadInfo& pbr_gltf_LoadInfo)
{
    materials.push_back(
        std::make_unique<PBRMaterial>(
            static_cast<MaterialID>(materials.size() - 1),
            pbr_gltf_LoadInfo.roughness,
            pbr_gltf_LoadInfo.metalic,
            textureManager.loadTexture(device, Texture::ALBEDO, pbr_gltf_LoadInfo.albedoMapPath),
            textureManager.loadTexture(device, Texture::AO, pbr_gltf_LoadInfo.aoMapPath),
            textureManager.loadTexture(device, Texture::METALIC_ROUGHNESS, pbr_gltf_LoadInfo.metalicRoughnessMapPath),
            textureManager.loadTexture(device, Texture::NORMAL, pbr_gltf_LoadInfo.normalMapPath),
            textureManager.loadTexture(device, Texture::DISPLACEMENT, pbr_gltf_LoadInfo.displacementMapPath)));
    return static_cast<MaterialID>(materials.size() - 1);
}