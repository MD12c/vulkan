#ifndef TEXTURE_CLASS_H
#define TEXTURE_CLASS_H

#include "vulkan/vulkan_core.h"
#include "..\vkBackend\Device.h"

class Texture
{
private:
    Device& device;

    VkImage        imageBuffer;
    VkImageView    imageView;
    VkDeviceMemory imageBufferMemory;
    VkSampler      sampler;

    void generateMipmaps(VkImage image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

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

    Texture(Device& device, TextureType textureType, unsigned char* bytes, int widthImg, int heightImg, int numColCh);
    ~Texture();
    Texture(const Texture&)             = delete;
    Texture& operator=(const Texture&)  = delete;
    Texture(Texture&& other)            = delete;
    Texture& operator=(Texture&& other) = delete;

    VkSampler   getSampler() { return sampler; }
    VkImageView getImageView() { return imageView; }
};

#endif