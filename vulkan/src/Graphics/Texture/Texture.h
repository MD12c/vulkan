#ifndef TEXTURE_CLASS_H
#define TEXTURE_CLASS_H

#include "vulkan/vulkan_core.h"
#include "..\vkBackend\Device.h"
#include "..\vkBackend\DescriptorSetsManager.h"

class Texture
{
private:
    Device& device;

    VkImage        imageBuffer;
    VkImageView    imageView;
    VkDeviceMemory imageBufferMemory;
    VkSampler      sampler;

public:

    enum TextureType
    {
        NONE,
        DIFFUSE,
        SPECULAR,
        ALBEDO,
        AO,
        METALIC_ROUGHNESS,
        NORMAL,
        DISPLACEMENT,
        CUSTOM
    } textureType;

    Texture(Device& device, TextureType textureType, VkDescriptorSet descriptorSet, DescriptorSetsManager& descriptorSetsManager, unsigned char* bytes, int widthImg, int heightImg);
    ~Texture();
    Texture(const Texture&)             = delete;
    Texture& operator=(const Texture&)  = delete;
    Texture(Texture&& other)            = delete;
    Texture& operator=(Texture&& other) = delete;
};

#endif