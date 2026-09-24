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
    DescriptorSetsManager&                                    descriptorSetsManager;
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures;  // TODO improve lookup

public:
    TextureManager(DescriptorSetsManager& descriptorSetsManager);

    std::shared_ptr<Texture> loadTexture(Device& device, std::string filePath);
};

#endif