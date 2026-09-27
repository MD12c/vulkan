#ifndef DESCRIPTORSETS_MANAGER_CLASS_H
#define DESCRIPTORSETS_MANAGER_CLASS_H

#include <array>
#include <cstddef>

#include "Device.h"
#include "Globals.h"

class DescriptorSetsManager
{
private:
    Device& device;

    VkDescriptorPool descriptorPool;

    VkDescriptorSetLayout globalSetLayout;  // ordered collection of VkDescriptorSetLayoutBinding
    VkDescriptorSetLayout textureSetLayout;

public:
    static constexpr size_t NUM_BUFFER_BINDINGS  = 1;
    static constexpr size_t NUM_SAMPLER_BINDINGS = 5;

    struct Descriptor
    {
        AllocatedBuffer                                            buffer;
        std::array<AllocatedBuffer, Globals::MAX_FRAMES_IN_FLIGHT> Buffers;
        std::array<VkDescriptorSet, Globals::MAX_FRAMES_IN_FLIGHT> descriptorSets;
    };
    std::array<Descriptor, NUM_BUFFER_BINDINGS> bufferDescriptors{};

    DescriptorSetsManager(Device& device);
    ~DescriptorSetsManager();
    DescriptorSetsManager(const DescriptorSetsManager&)             = delete;
    DescriptorSetsManager& operator=(const DescriptorSetsManager&)  = delete;
    DescriptorSetsManager(const DescriptorSetsManager&&)            = delete;
    DescriptorSetsManager& operator=(const DescriptorSetsManager&&) = delete;

    void allocDescriptor(size_t bufferSize, int descriptorIndex);

    VkDescriptorSetLayout& getGlobalSetLayouts() { return globalSetLayout; }
    VkDescriptorSetLayout& getTextureSetLayout() { return textureSetLayout; }
    VkDescriptorPool&      getDescriptorPool() { return descriptorPool; }
};

#endif