#include "Texture.h"

Texture::Texture(Device& device, TextureType textureType, VkDescriptorSet descriptorSet, DescriptorSetsManager& descriptorSetsManager, unsigned char* bytes, int widthImg, int heightImg)
    : device(device), textureType(textureType)
{
    void*          pixel_ptr = bytes;
    VkBuffer       tempImageBuffer;
    VkDeviceMemory tempImageBufferMemory;

    {
        VkMemoryPropertyFlags propertiesBuffer = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        VkDeviceSize          imageSize        = static_cast<VkDeviceSize>(widthImg) * static_cast<VkDeviceSize>(heightImg) * 4;

        device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, propertiesBuffer, tempImageBuffer, tempImageBufferMemory);

        {
            void* data;
            vkMapMemory(device.device(), tempImageBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, pixel_ptr, static_cast<size_t>(imageSize));
            vkUnmapMemory(device.device(), tempImageBufferMemory);
        }
    }

    {
        VkMemoryPropertyFlags propertiesImage = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        VkImageCreateInfo imageInfo{};
        imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType     = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width  = widthImg;
        imageInfo.extent.height = heightImg;
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = 1;
        imageInfo.arrayLayers   = 1;
        imageInfo.format        = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

        device.createImageWithInfo(imageInfo, propertiesImage, imageBuffer, imageBufferMemory);
    }

    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image                           = imageBuffer;
        viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format                          = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        vkCreateImageView(device.device(), &viewInfo, nullptr, &imageView);
    }

    {
        VkImageSubresourceRange range;
        range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseMipLevel   = 0;
        range.levelCount     = 1;
        range.baseArrayLayer = 0;
        range.layerCount     = 1;

        device.transitionImageLayout(imageBuffer, range, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        device.copyBufferToImage(tempImageBuffer, imageBuffer, widthImg, heightImg, 1);

        device.transitionImageLayout(imageBuffer, range, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    vkDestroyBuffer(device.device(), tempImageBuffer, nullptr);
    vkFreeMemory(device.device(), tempImageBufferMemory, nullptr);

    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter               = VK_FILTER_LINEAR;
        samplerInfo.minFilter               = VK_FILTER_LINEAR;
        samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable        = VK_TRUE;  // for me
        samplerInfo.maxAnisotropy           = 1.0f;
        samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias              = 0.0f;
        samplerInfo.minLod                  = 0.0f;
        samplerInfo.maxLod                  = 0.0f;

        vkCreateSampler(device.device(), &samplerInfo, nullptr, &sampler);
    }

    {

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView   = imageView;
        imageInfo.sampler     = sampler;

        VkWriteDescriptorSet write{};
        write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet          = descriptorSet;
        write.dstBinding      = 0;
        write.descriptorCount = 1;
        write.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo      = &imageInfo;

        vkUpdateDescriptorSets(device.device(), 1, &write, 0, nullptr);
    }
}

Texture::~Texture()
{
    vkDestroyImageView(device.device(), imageView, nullptr);
    vkDestroyImage(device.device(), imageBuffer, nullptr);
    vkFreeMemory(device.device(), imageBufferMemory, nullptr);
    vkDestroySampler(device.device(), sampler, nullptr);
}