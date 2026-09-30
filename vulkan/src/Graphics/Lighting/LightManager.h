#ifndef SHADOW_SYSTEM_CLASS_H
#define SHADOW_SYSTEM_CLASS_H

#include <array>
#include <cstdint>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "Globals.h"
#include "../vkBackend/Device.h"
#include "../vkBackend/DescriptorSetsManager.h"

#include "DirectionLight.h"
// #include "SpotLight.h"
// #include "PointLight.h"

class LightManager
{
private:
    Device&                device;
    DescriptorSetsManager& descriptorSetsManager;

    struct LightTypeResources
    {
        AllocatedImage             imageArray;
        VkSampler                  sampler;
        std::vector<VkFramebuffer> framebuffers;
        std::vector<VkImageView>   layerViews;

        std::array<AllocatedBuffer, Globals::MAX_FRAMES_IN_FLIGHT> buffers;
        std::array<VkDescriptorSet, Globals::MAX_FRAMES_IN_FLIGHT> descriptorSets;
    };

public:
    LightTypeResources dir;

    LightManager(Device& device, VkFormat depthFormat, VkRenderPass renderPass, DescriptorSetsManager& descriptorSetsManager);
    ~LightManager();

    void ExportUniformsTo(int frameIndex, const std::vector<DirectionLight>& directionLights) const;

    int addDirectionLight(DirectionLight::DirectionLightInfo& createInfo, std::vector<DirectionLight>& directionLights)
    {
        uint32_t index = static_cast<uint32_t>(directionLights.size());
        directionLights.emplace_back(index, createInfo);
        return index;
    }
};

#endif