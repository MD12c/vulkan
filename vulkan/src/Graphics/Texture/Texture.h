#ifndef TEXTURE_CLASS_H
#define TEXTURE_CLASS_H

#include "vulkan/vulkan_core.h"
#include "../vkBackend/Device.h"
#include "../vkBackend/DescriptorSetsManager.h"

class Texture
{
private:
    Device& device;

    VkImage        imageBuffer;
    VkImageView    imageView;
    VkDeviceMemory imageBufferMemory;
    VkSampler      sampler;

public:
    VkDescriptorSet descriptorSet;

    Texture(Device& device, DescriptorSetsManager& descriptorSetsManager, unsigned char* bytes, int widthImg, int heightImg);
    ~Texture();
};

#endif