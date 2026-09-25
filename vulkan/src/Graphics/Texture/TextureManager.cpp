#include "TextureManager.h"

#include "Graphics/Texture/Texture.h"
#include "stb/stb_image.h"
#include "../vkBackend/Device.h"

TextureManager::TextureManager(Device& device, DescriptorSetsManager& descriptorSetsManager)
    : device(device), descriptorSetsManager(descriptorSetsManager)
{
    unsigned char white[4] = { 255, 255, 255, 255 };
    defaultWhite           = std::make_shared<Texture>(device, Texture::ALBEDO, white, 1, 1, 4);

    unsigned char blue[4] = { 128, 128, 255, 255 };
    defaultBlue           = std::make_shared<Texture>(device, Texture::NORMAL, blue, 1, 1, 4);

    unsigned char black[4] = { 0, 0, 0, 255 };
    defaultBlack           = std::make_shared<Texture>(device, Texture::DISPLACEMENT, black, 1, 1, 4);

    missingAlbedo = loadTexture(device, Texture::ALBEDO, "Assets/Textures/defaultFallback/missingAlbedo.png");
}

std::shared_ptr<Texture> TextureManager::loadTexture(Device& device, Texture::TextureType textureType, std::string filePath)
{
    if (textureType == Texture::NONE)
        return nullptr;

    int widthImg, heightImg, numColCh;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* bytes = stbi_load(filePath.c_str(), &widthImg, &heightImg, &numColCh, STBI_rgb_alpha);

    if (!bytes)
        return nullptr;

    auto it_type = loadedTextures.find(textureType);
    if (it_type == loadedTextures.end())
    {
        std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(device, textureType, bytes, widthImg, heightImg, numColCh);
        loadedTextures[textureType]         = { { filePath, newTexture } };
        stbi_image_free(bytes);
        return newTexture;
    }

    auto it_path = it_type->second.find(filePath);
    if (it_path == it_type->second.end())
    {
        std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(device, textureType, bytes, widthImg, heightImg, numColCh);
        it_type->second[filePath]           = newTexture;
        stbi_image_free(bytes);
        return newTexture;
    }

    stbi_image_free(bytes);
    return it_type->second[filePath];
}