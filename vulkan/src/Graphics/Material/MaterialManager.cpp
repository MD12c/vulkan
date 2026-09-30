#include "MaterialManager.h"

#include <array>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <memory>

#include "Graphics/Texture/Texture.h"
#include "IMaterial.h"
#include "PBRMaterial.h"

MaterialManager::MaterialManager(Device& device, TextureManager& textureManager, DescriptorSetsManager& descriptorSetsManager)
    : device(device), textureManager(textureManager), descriptorSetsManager(descriptorSetsManager)
{
}

MaterialManager::~MaterialManager()
{
}

MaterialID MaterialManager::LoadMaterialPBRgltf(MaterialManager::PBR_GLTF_LoadInfo& pbr_gltf_LoadInfo)
{
    materials.push_back(
        std::make_unique<PBRMaterial>(
            static_cast<MaterialID>(materials.size()),
            pbr_gltf_LoadInfo.roughness,
            pbr_gltf_LoadInfo.metalic,
            textureManager.loadTexture(device, Texture::ALBEDO, pbr_gltf_LoadInfo.albedoMapPath),
            textureManager.loadTexture(device, Texture::AO, pbr_gltf_LoadInfo.aoMapPath),
            textureManager.loadTexture(device, Texture::METALIC_ROUGHNESS, pbr_gltf_LoadInfo.metalicRoughnessMapPath),
            textureManager.loadTexture(device, Texture::NORMAL, pbr_gltf_LoadInfo.normalMapPath),
            textureManager.loadTexture(device, Texture::DISPLACEMENT, pbr_gltf_LoadInfo.displacementMapPath)));

    descriptorSetsManager.allocateSet(descriptorSetsManager.getTextureSetLayout(), materials.back()->descriptorSet);

    PBRMaterial* pbr_ptr = reinterpret_cast<PBRMaterial*>(materials.back().get());

    std::array<std::shared_ptr<Texture>, PBRMaterial::NUM_TEXTURES> textures = {
        pbr_ptr->albedoMap, pbr_ptr->aoMap, pbr_ptr->metalicRoughnessMap, pbr_ptr->normalMap, pbr_ptr->displacementMap
    };
    std::array<std::shared_ptr<Texture>, PBRMaterial::NUM_TEXTURES> defaultTextures = {
        textureManager.missingAlbedo, textureManager.defaultWhite, textureManager.defaultBlack, textureManager.defaultBlue, textureManager.defaultBlack
    };
    std::array<VkDescriptorImageInfo, PBRMaterial::NUM_TEXTURES> imageInfos{};
    std::array<VkWriteDescriptorSet, PBRMaterial::NUM_TEXTURES>  writes{};

    for (uint32_t i = 0; i < 5; i++)
    {
        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfos[i].imageView   = textures[i] ? textures[i]->getImageView() : defaultTextures[i]->getImageView();
        imageInfos[i].sampler     = textures[i] ? textures[i]->getSampler() : defaultTextures[i]->getSampler();

        writes[i].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet          = materials.back()->descriptorSet;
        writes[i].dstBinding      = i;
        writes[i].descriptorCount = 1;
        writes[i].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[i].pImageInfo      = &imageInfos[i];
    }

    vkUpdateDescriptorSets(device.device(), 5, writes.data(), 0, nullptr);

    return static_cast<MaterialID>(materials.size() - 1);
}