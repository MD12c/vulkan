#include "DescriptorSetsManager.h"

#include <stdexcept>
#include <array>

#include "Device.h"

DescriptorSetsManager::DescriptorSetsManager(Device& device)
    : device(device)
{
    {  // Pool
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
    {  // Camera buffer
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
    {  // Samplers
        std::array<VkDescriptorSetLayoutBinding, NUM_SAMPLER_BINDINGS> samplerBindings{};
        for (uint32_t i = 0; i < NUM_SAMPLER_BINDINGS; i++)
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
    {  // shadowMap buffer
        std::array<VkDescriptorSetLayoutBinding, NUM_SHADOW_BINDINGS> shadowBufferBinding{};
        for (uint32_t i = 0; i < 1; i++)  //! change for future light types
        {
            shadowBufferBinding[i].binding         = i;
            shadowBufferBinding[i].descriptorCount = 1;
            shadowBufferBinding[i].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            shadowBufferBinding[i].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        for (uint32_t i = 1; i < NUM_SHADOW_BINDINGS; i++)  //! change for future light types
        {
            shadowBufferBinding[i].binding         = i;
            shadowBufferBinding[i].descriptorCount = 1;
            shadowBufferBinding[i].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            shadowBufferBinding[i].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        VkDescriptorSetLayoutCreateInfo setinfo{};
        setinfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        setinfo.pNext        = nullptr;
        setinfo.flags        = 0;
        setinfo.bindingCount = static_cast<uint32_t>(NUM_SHADOW_BINDINGS);
        setinfo.pBindings    = shadowBufferBinding.data();

        if (vkCreateDescriptorSetLayout(device.device(), &setinfo, nullptr, &shadowMapSetLayout))
            throw std::runtime_error("[ERROR] Failed to create DescriptorSetLayout");
    }
}

DescriptorSetsManager::~DescriptorSetsManager()
{
    vkDestroyDescriptorSetLayout(device.device(), globalSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device.device(), textureSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device.device(), shadowMapSetLayout, nullptr);
    vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);
}

void DescriptorSetsManager::allocateSet(VkDescriptorSetLayout layout, VkDescriptorSet& descriptorSet)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.pNext              = nullptr;
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts        = &layout;

    if (vkAllocateDescriptorSets(device.device(), &allocInfo, &descriptorSet))
        throw std::runtime_error("[ERROR] Failed to allocate descriptor");
}

void DescriptorSetsManager::writeUniformBuffer(VkDescriptorSet set, uint32_t binding, VkBuffer buffer, VkDeviceSize bufferSize)
{
    VkDescriptorBufferInfo binfo{};
    binfo.buffer = buffer;
    binfo.offset = 0;
    binfo.range  = bufferSize;

    VkWriteDescriptorSet setWrite{};
    setWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    setWrite.pNext           = nullptr;
    setWrite.dstBinding      = binding;
    setWrite.dstSet          = set;
    setWrite.descriptorCount = 1;
    setWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    setWrite.pBufferInfo     = &binfo;

    vkUpdateDescriptorSets(device.device(), 1, &setWrite, 0, nullptr);
}
