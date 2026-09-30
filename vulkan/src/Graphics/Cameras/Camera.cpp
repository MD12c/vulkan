#include "Camera.h"

#include <vulkan/vulkan_core.h>

#include "../Window.h"
#include "../vkBackend/Device.h"
#include "Globals.h"

Camera::Camera(Device& device, Window& window, DescriptorSetsManager& descriptorSetManager)
    : device(device), window(window)
{
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage         = VMA_MEMORY_USAGE_AUTO;
    allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    allocInfo.flags         = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    for (auto& Buffer : cameraDescriptors.buffers)
        device.createBuffer(sizeof(Payload), bufferUsage, allocInfo, Buffer);

    for (auto& descriptorSet : cameraDescriptors.descriptorSets)
        descriptorSetManager.allocateSet(descriptorSetManager.getGlobalSetLayout(), descriptorSet);

    for (int i = 0; i < Globals::MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = cameraDescriptors.buffers[i].buffer;
        bufferInfo.offset = 0;
        bufferInfo.range  = payloadSize;

        VkWriteDescriptorSet setWrite{};
        setWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        setWrite.pNext           = nullptr;
        setWrite.dstBinding      = 0;
        setWrite.dstSet          = cameraDescriptors.descriptorSets[i];
        setWrite.descriptorCount = 1;
        setWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        setWrite.pBufferInfo     = &bufferInfo;

        vkUpdateDescriptorSets(device.device(), 1, &setWrite, 0, nullptr);
    }
}

Camera::~Camera()
{
    for (auto& Buffer : cameraDescriptors.buffers)
        vmaDestroyBuffer(device.getVMA(), Buffer.buffer, Buffer.allocation);
}

void Camera::updateUniforms(VmaAllocation& allocation) const
{
    Payload payload(proj, view, Position);
    void*   data;
    vmaMapMemory(device.getVMA(), allocation, &data);
    memcpy(data, (void*)&payload, static_cast<size_t>(payloadSize));
    vmaUnmapMemory(device.getVMA(), allocation);
}

glm::vec2 Camera::screenToWorld(const glm::vec2& pos)
{
    float ndcX = (2.0f * (float)pos.x) / window.getExtent().width - 1.0f;
    float ndcY = 1.0f - (2.0f * (float)pos.y) / window.getExtent().height;

    glm::mat4 invCamera = glm::inverse(proj * view);
    glm::vec4 world     = invCamera * glm::vec4(ndcX, ndcY, 0.0f, 1.0f);

    if (world.w != 0.0f)
        world /= world.w;

    return glm::vec2(world.x, world.y);
}

glm::mat4 Camera::getRotationMat()
{
    glm::mat4 noTransView = glm::mat4(glm::mat3(view));
    return proj * noTransView;
}