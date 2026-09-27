#include "DescriptorSetsManager.h"

#include "../Cameras/Camera.h"
#include "Device.h"

#include <stdexcept>

DescriptorSetsManager::DescriptorSetsManager(Device& device)
    : device(device)
{
    // Pool
    {
        std::vector<VkDescriptorPoolSize> sizes = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },  // create a descriptor pool that will hold 10 uniform buffers
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 }
        };

        VkDescriptorPoolCreateInfo pool_info{};
        pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags         = 0;
        pool_info.maxSets       = 110;
        pool_info.poolSizeCount = (uint32_t)sizes.size();
        pool_info.pPoolSizes    = sizes.data();

        if (vkCreateDescriptorPool(device.device(), &pool_info, nullptr, &descriptorPool))
            throw std::runtime_error("[ERROR] Failed to create DescriptorPool");
    }

    // Camera buffer
    {
        std::array<VkDescriptorSetLayoutBinding, NUM_BUFFER_BINDINGS> bufferBinding{};
        bufferBinding[0].binding         = 0;
        bufferBinding[0].descriptorCount = 1;
        bufferBinding[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bufferBinding[0].stageFlags      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo setinfo{};
        setinfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        setinfo.pNext        = nullptr;
        setinfo.flags        = 0;
        setinfo.bindingCount = static_cast<uint32_t>(NUM_BUFFER_BINDINGS);
        setinfo.pBindings    = bufferBinding.data();

        if (vkCreateDescriptorSetLayout(device.device(), &setinfo, nullptr, &globalSetLayout))
            throw std::runtime_error("[ERROR] Failed to create DescriptorSetLayout");
    }

    allocDescriptor(Camera::payloadSize, 0);

    // Samplers
    {
        std::array<VkDescriptorSetLayoutBinding, NUM_SAMPLER_BINDINGS> samplerBindings{};
        for (uint32_t i = 0; i < 5; i++)
        {
            samplerBindings[i].binding         = i;
            samplerBindings[i].descriptorCount = 1;
            samplerBindings[i].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerBindings[i].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        VkDescriptorSetLayoutCreateInfo setinfo{};
        setinfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        setinfo.bindingCount = static_cast<uint32_t>(NUM_SAMPLER_BINDINGS);
        setinfo.pBindings    = samplerBindings.data();

        if (vkCreateDescriptorSetLayout(device.device(), &setinfo, nullptr, &textureSetLayout))
            throw std::runtime_error("[ERROR] Failed to create texture DescriptorSetLayout");
    }
}

DescriptorSetsManager::~DescriptorSetsManager()
{
    vkDestroyDescriptorSetLayout(device.device(), globalSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device.device(), textureSetLayout, nullptr);
    vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);

    for (const auto& descriptor : bufferDescriptors)
    {
        for (const auto& Buffer : descriptor.Buffers)
            vmaDestroyBuffer(device.getVMA(), Buffer.buffer, Buffer.allocation);
    }
}

void DescriptorSetsManager::allocDescriptor(size_t bufferSize, int descriptorIndex)
{
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage         = VMA_MEMORY_USAGE_AUTO;
    allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    allocInfo.flags         = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    for (int i = 0; i < Globals::MAX_FRAMES_IN_FLIGHT; i++)
    {
        device.createBuffer(bufferSize, bufferUsage, allocInfo, bufferDescriptors[descriptorIndex].Buffers[i]);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.pNext              = nullptr;
        allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool     = descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts        = &globalSetLayout;

        if (vkAllocateDescriptorSets(device.device(), &allocInfo, &bufferDescriptors[descriptorIndex].descriptorSets[i]))
            throw std::runtime_error("[ERROR] Failed to allocate descriptor");

        VkDescriptorBufferInfo binfo{};
        binfo.buffer = bufferDescriptors[descriptorIndex].Buffers[i].buffer;
        binfo.offset = 0;
        binfo.range  = bufferSize;

        VkWriteDescriptorSet setWrite{};
        setWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        setWrite.pNext           = nullptr;
        setWrite.dstBinding      = 0;
        setWrite.dstSet          = bufferDescriptors[descriptorIndex].descriptorSets[i];
        setWrite.descriptorCount = 1;
        setWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        setWrite.pBufferInfo     = &binfo;

        vkUpdateDescriptorSets(device.device(), 1, &setWrite, 0, nullptr);
    }
}
