#include "TextureManager.h"

#include <stdexcept>

#include "Graphics/Texture/Texture.h"
#include "stb/stb_image.h"
#include "../vkBackend/Device.h"

TextureManager::TextureManager(Device& device, DescriptorSetsManager& descriptorSetsManager)
    : device(device), descriptorSetsManager(descriptorSetsManager)
{
    VkDescriptorSetLayout textureLayout = descriptorSetsManager.getTextureSetLayout();

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = descriptorSetsManager.getDescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts        = &textureLayout;

    vkAllocateDescriptorSets(device.device(), &allocInfo, &descriptorSet);
}

std::shared_ptr<Texture> TextureManager::loadTexture(Device& device, Texture::TextureType textureType, std::string filePath)
{
    if (textureType == Texture::NONE)
        return nullptr;

    int widthImg, heightImg, numColCh;  // TODO auto determine color channels
    stbi_set_flip_vertically_on_load(false);
    unsigned char* bytes = stbi_load(filePath.c_str(), &widthImg, &heightImg, &numColCh, STBI_rgb_alpha);

    if (!bytes)
        return nullptr;

    auto it_type = loadedTextures.find(textureType);
    if (it_type == loadedTextures.end())
    {
        std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(device, textureType, descriptorSet, descriptorSetsManager, bytes, widthImg, heightImg);
        loadedTextures[textureType]         = { { filePath, newTexture } };
        stbi_image_free(bytes);
        return newTexture;
    }

    auto it_path = it_type->second.find(filePath);
    if (it_path == it_type->second.end())
    {
        std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(device, textureType, descriptorSet, descriptorSetsManager, bytes, widthImg, heightImg);
        it_type->second[filePath]           = newTexture;
        stbi_image_free(bytes);
        return newTexture;
    }

    stbi_image_free(bytes);
    return it_type->second[filePath];
}