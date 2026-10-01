#include "LightManager.h"
#include <vulkan/vulkan_core.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <glm/gtx/rotate_vector.hpp>
#include <stdexcept>
#include <cstring>

#include "DirectionLight.h"
#include "Globals.h"
#include "Graphics/vkBackend/DescriptorSetsManager.h"
#include "ShadowMapDimensions.h"
// #include "../Material/MaterialManager.h"
// #include "../Model/BasicShapes.h"

LightManager::LightManager(Device& device, VkFormat depthFormat, VkRenderPass renderPass, DescriptorSetsManager& descriptorSetsManager)
    : device(device), descriptorSetsManager(descriptorSetsManager)
{
    {  // imageArray creation
        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage         = VMA_MEMORY_USAGE_AUTO;
        allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        VkImageCreateInfo imageInfo{};
        imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType     = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width  = ShadowMapDimensions::SHADOW_MAP_WIDTH;
        imageInfo.extent.height = ShadowMapDimensions::SHADOW_MAP_HEIGHT;
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = 1;
        imageInfo.arrayLayers   = ShadowMapDimensions::MAX_DIR_LIGHTS;
        imageInfo.format        = depthFormat;
        imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

        device.createImageWithInfo(imageInfo, allocInfo, dir.imageArray);
    }
    {  // imageArray view creation
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image                           = dir.imageArray.image;
        viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        viewInfo.format                          = depthFormat;
        viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = ShadowMapDimensions::MAX_DIR_LIGHTS;

        if (vkCreateImageView(device.device(), &viewInfo, nullptr, &dir.imageArray.imageView))
            throw std::runtime_error("[ERROR] failed to create image view");
    }
    {  // layout transfer
        VkImageSubresourceRange fullRange{};
        fullRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
        fullRange.baseMipLevel   = 0;
        fullRange.levelCount     = 1;
        fullRange.baseArrayLayer = 0;
        fullRange.layerCount     = ShadowMapDimensions::MAX_DIR_LIGHTS;

        device.transitionImageLayout(dir.imageArray.image, fullRange, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    {  // layers views creation
        dir.layerViews.resize(ShadowMapDimensions::MAX_DIR_LIGHTS);
        for (uint32_t i = 0; i < ShadowMapDimensions::MAX_DIR_LIGHTS; i++)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image                           = dir.imageArray.image;
            viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format                          = depthFormat;
            viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel   = 0;
            viewInfo.subresourceRange.levelCount     = 1;
            viewInfo.subresourceRange.baseArrayLayer = i;
            viewInfo.subresourceRange.layerCount     = 1;

            if (vkCreateImageView(device.device(), &viewInfo, nullptr, &dir.layerViews[i]))
                throw std::runtime_error("[ERROR] failed to create image view");
        }
    }
    {  // framebuffer creation
        dir.framebuffers.resize(ShadowMapDimensions::MAX_DIR_LIGHTS);
        for (uint32_t i = 0; i < ShadowMapDimensions::MAX_DIR_LIGHTS; i++)
        {
            VkFramebufferCreateInfo frameBufferInfo{};
            frameBufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            frameBufferInfo.width           = ShadowMapDimensions::SHADOW_MAP_WIDTH;
            frameBufferInfo.height          = ShadowMapDimensions::SHADOW_MAP_HEIGHT;
            frameBufferInfo.layers          = 1;
            frameBufferInfo.attachmentCount = 1;
            frameBufferInfo.pAttachments    = &dir.layerViews[i];
            frameBufferInfo.renderPass      = renderPass;

            if (vkCreateFramebuffer(device.device(), &frameBufferInfo, nullptr, &dir.framebuffers[i]))
                throw std::runtime_error("[ERROR] failed to create framebuffer");
        }
    }
    {  // image sampler
        VkPhysicalDeviceProperties props = device.getPhysicalDevicePropreties();

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter               = VK_FILTER_LINEAR;
        samplerInfo.minFilter               = VK_FILTER_LINEAR;
        samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.anisotropyEnable        = VK_FALSE;
        samplerInfo.maxAnisotropy           = props.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod                  = 0;
        samplerInfo.maxLod                  = 0;

        if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &dir.sampler))
            throw std::runtime_error("[ERROR] failed to create sampler");
    }
    {  // buffers creation
        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage         = VMA_MEMORY_USAGE_AUTO;
        allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        allocInfo.flags         = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

        for (uint32_t i = 0; i < Globals::MAX_FRAMES_IN_FLIGHT; i++)
            device.createBuffer(sizeof(DirectionLight::DirLightsUBO), bufferUsage, allocInfo, dir.buffers[i]);
    }
    {  // descriptor sets init
        for (uint32_t i = 0; i < Globals::MAX_FRAMES_IN_FLIGHT; i++)
        {
            descriptorSetsManager.allocateSet(descriptorSetsManager.getShadowSetLayout(), dir.descriptorSets[i]);

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = dir.buffers[i].buffer;
            bufferInfo.offset = 0;
            bufferInfo.range  = sizeof(DirectionLight::DirLightsUBO);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView   = dir.imageArray.imageView;
            imageInfo.sampler     = dir.sampler;

            std::array<VkWriteDescriptorSet, 2> writes{};

            writes[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[0].dstSet          = dir.descriptorSets[i];
            writes[0].dstBinding      = 0;
            writes[0].dstArrayElement = 0;
            writes[0].descriptorCount = 1;
            writes[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writes[0].pBufferInfo     = &bufferInfo;

            writes[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[1].dstSet          = dir.descriptorSets[i];
            writes[1].dstBinding      = 1;
            writes[1].dstArrayElement = 0;
            writes[1].descriptorCount = 1;
            writes[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writes[1].pImageInfo      = &imageInfo;

            vkUpdateDescriptorSets(device.device(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
        }
    }
}

LightManager::~LightManager()
{
    for (auto& frameBuffer : dir.framebuffers)
        vkDestroyFramebuffer(device.device(), frameBuffer, nullptr);

    for (auto& layerView : dir.layerViews)
        vkDestroyImageView(device.device(), layerView, nullptr);

    vkDestroyImageView(device.device(), dir.imageArray.imageView, nullptr);

    vmaDestroyImage(device.getVMA(), dir.imageArray.image, dir.imageArray.allocation);

    for (uint32_t i = 0; i < Globals::MAX_FRAMES_IN_FLIGHT; i++)
        vmaDestroyBuffer(device.getVMA(), dir.buffers[i].buffer, dir.buffers[i].allocation);

    vkDestroySampler(device.device(), dir.sampler, nullptr);
}

void LightManager::ExportUniformsTo(int frameIndex, const std::vector<DirectionLight>& directionLights) const
{
    DirectionLight::DirLightsUBO dirUBO{};

    if (directionLights.size() > ShadowMapDimensions::MAX_DIR_LIGHTS)
        throw std::runtime_error("[ERROR] max number of direction lights overflow");

    dirUBO.numDirLights = static_cast<uint32_t>(directionLights.size());
    for (size_t i = 0; i < directionLights.size(); i++)
    {
        dirUBO.direction[i]    = glm::vec4(directionLights[i].getDirection(), 0.0f);
        dirUBO.color[i]        = glm::vec4(directionLights[i].getColor(), 0.0f);
        dirUBO.layerIndex[i]   = glm::ivec4(directionLights[i].layerIndex);
        dirUBO.shadowMatrix[i] = directionLights[i].getShadowMatrix();
    }

    void* data;
    vmaMapMemory(device.getVMA(), dir.buffers[frameIndex].allocation, &data);
    memcpy(data, &dirUBO, sizeof(dirUBO));
    vmaUnmapMemory(device.getVMA(), dir.buffers[frameIndex].allocation);
}
