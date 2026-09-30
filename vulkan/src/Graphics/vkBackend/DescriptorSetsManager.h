#ifndef DESCRIPTORSETS_MANAGER_CLASS_H
#define DESCRIPTORSETS_MANAGER_CLASS_H

#include <cstddef>

#include "Device.h"

class DescriptorSetsManager
{
private:
    Device& device;

    VkDescriptorPool descriptorPool;

    VkDescriptorSetLayout globalSetLayout;  // ordered collection of VkDescriptorSetLayoutBinding
    VkDescriptorSetLayout textureSetLayout;
    VkDescriptorSetLayout shadowMapSetLayout;

public:
    static constexpr size_t NUM_BUFFER_BINDINGS  = 1;
    static constexpr size_t NUM_SAMPLER_BINDINGS = 5;
    static constexpr size_t NUM_SHADOW_BINDINGS  = 2;  //! change for future light types to 6

    DescriptorSetsManager(Device& device);
    ~DescriptorSetsManager();
    DescriptorSetsManager(const DescriptorSetsManager&)             = delete;
    DescriptorSetsManager& operator=(const DescriptorSetsManager&)  = delete;
    DescriptorSetsManager(const DescriptorSetsManager&&)            = delete;
    DescriptorSetsManager& operator=(const DescriptorSetsManager&&) = delete;

    void allocateSet(VkDescriptorSetLayout layout, VkDescriptorSet& descriptorSet);
    void writeUniformBuffer(VkDescriptorSet set, uint32_t binding, VkBuffer buffer, VkDeviceSize bufferSize);

    const VkDescriptorSetLayout& getGlobalSetLayout() const { return globalSetLayout; }
    const VkDescriptorSetLayout& getTextureSetLayout() const { return textureSetLayout; }
    const VkDescriptorSetLayout& getShadowSetLayout() const { return shadowMapSetLayout; }
    const VkDescriptorPool&      getDescriptorPool() const { return descriptorPool; }
};

#endif