#ifndef DESCRIPTORSETS_MANAGER_CLASS_H
#define DESCRIPTORSETS_MANAGER_CLASS_H

#include <array>
#include <vector>

#include "Device.h"
#include "Globals.h"

class DescriptorSetsManager
{
private:
    Device& device;

    VkDescriptorSetLayout globalSetLayout;
    VkDescriptorPool      descriptorPool;

public:
    static constexpr size_t                                       NUM_BUFFER_BINDINGS = 1;
    std::array<VkDescriptorSetLayoutBinding, NUM_BUFFER_BINDINGS> bufferBinding{};

    struct Descriptor
    {
        struct Buffer
        {
            VkBuffer       buffer;
            VkDeviceMemory bufferMemory;
        };
        std::array<Buffer, Globals::MAX_FRAMES_IN_FLIGHT>          Buffers;
        std::array<VkDescriptorSet, Globals::MAX_FRAMES_IN_FLIGHT> descriptorSets;
    };
    std::array<Descriptor, NUM_BUFFER_BINDINGS> descriptors;

    DescriptorSetsManager(Device& device);
    ~DescriptorSetsManager();
    DescriptorSetsManager(const DescriptorSetsManager&)             = delete;
    DescriptorSetsManager& operator=(const DescriptorSetsManager&)  = delete;
    DescriptorSetsManager(const DescriptorSetsManager&&)            = delete;
    DescriptorSetsManager& operator=(const DescriptorSetsManager&&) = delete;

    void allocDescriptor(size_t bufferSize, int descriptorIndex);

    VkDescriptorSetLayout& getVkDescriptorSetLayout() { return globalSetLayout; }
};

#endif