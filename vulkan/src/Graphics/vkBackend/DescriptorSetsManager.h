#ifndef DESCRIPTORSETS_MANAGER_CLASS_H
#define DESCRIPTORSETS_MANAGER_CLASS_H

#include <array>

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

    static constexpr size_t                                       NUM_BUFFER_BINDINGS = 1;
    std::array<VkDescriptorSetLayoutBinding, NUM_BUFFER_BINDINGS> bufferBinding{};  // binding number, type
    std::array<Descriptor, NUM_BUFFER_BINDINGS>                   bufferDescriptors{};

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