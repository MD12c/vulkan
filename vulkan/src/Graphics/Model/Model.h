#ifndef MODEL_CLASS_H
#define MODEL_CLASS_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "..\vkBackend\Device.h"

class Model
{
public:
    struct Vertex
    {
        glm::vec2 pos;
        glm::vec3 color;

        static std::vector<VkVertexInputBindingDescription>   getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

private:
    Device&        device;
    VkBuffer       vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    uint32_t       vertexCount;

    void createVertexBuffers(const std::vector<Vertex>& vertices);

public:
    Model(Device& device, const std::vector<Vertex>& vertices);
    ~Model();
    Model(const Model&)            = delete;
    Model& operator=(const Model&) = delete;

    void Bind(VkCommandBuffer commandBuffer);
    void Draw(VkCommandBuffer commandBuffer);
};

#endif