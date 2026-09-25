#ifndef TEXTUREMANAGER_CLASS_H
#define TEXTUREMANAGER_CLASS_H

#include <unordered_map>
#include <memory>
#include <string>

#include "Texture.h"
#include "../vkBackend/DescriptorSetsManager.h"

class Device;

class TextureManager
{
private:
    Device&                device;
    DescriptorSetsManager& descriptorSetsManager;

    std::unordered_map<Texture::TextureType, std::unordered_map<std::string, std::shared_ptr<Texture>>> loadedTextures;  // TODO improve lookup

public:
    std::shared_ptr<Texture> defaultWhite;
    std::shared_ptr<Texture> defaultBlue;
    std::shared_ptr<Texture> defaultBlack;
    std::shared_ptr<Texture> missingAlbedo;

    TextureManager(Device& device, DescriptorSetsManager& descriptorSetsManager);

    std::shared_ptr<Texture> loadTexture(Device& device, Texture::TextureType textureType, std::string filePath);
};

#endif