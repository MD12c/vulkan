#include "TextureManager.h"

#include <stdexcept>

#include "stb/stb_image.h"
#include "../vkBackend/Device.h"

TextureManager::TextureManager(DescriptorSetsManager& descriptorSetsManager)
    : descriptorSetsManager(descriptorSetsManager)
{
}

std::shared_ptr<Texture> TextureManager::loadTexture(Device& device, std::string filePath)
{
    int widthImg, heightImg, numColCh;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* bytes = stbi_load(filePath.c_str(), &widthImg, &heightImg, &numColCh, STBI_rgb_alpha);

    if (bytes == nullptr)
        throw std::runtime_error("[ERROR] Couldn't Load Texture");

    auto it = textures.find(filePath);

    if (it == textures.end())
    {
        std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(device, descriptorSetsManager, bytes, widthImg, heightImg);
        textures[filePath]                  = newTexture;
        stbi_image_free(bytes);
        return newTexture;
    }
    else
    {
        stbi_image_free(bytes);
        return it->second;
    }
}