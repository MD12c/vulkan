#include "Camera.h"

#include "vulkan/vulkan_core.h"
#include "../Window.h"

Camera::Camera(Device& device, Window& window) : device(device), window(window)
{
}

void Camera::updateUniforms(VkDeviceMemory& bufferMemory) const
{
    Payload payload(proj, view, Position);  // TODO try glm::inverse(view) after
    void*   data;
    vkMapMemory(device.device(), bufferMemory, 0, payloadSize, 0, &data);
    memcpy(data, (void*)&payload, static_cast<size_t>(payloadSize));
    vkUnmapMemory(device.device(), bufferMemory);
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