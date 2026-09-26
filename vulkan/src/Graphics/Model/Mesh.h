#ifndef MESH_CLASS_H
#define MESH_CLASS_H

#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/matrix_float4x4.hpp>

#include "vulkan/vulkan_core.h"

#include "../vkBackend/Device.h"
#include "../Material/IMaterial.h"

struct alignas(16) Vertex
{
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec2 texUV{};
    glm::vec3 tangent{};

    static std::vector<VkVertexInputBindingDescription>   getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};

class Mesh
{
private:
    Device&        device;
    VkBuffer       vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer       indexBuffer;
    VkDeviceMemory indexBufferMemory;

public:
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;
    MaterialID            materialID;

    // BoundingSphere sphere;

    Mesh(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, MaterialID materialID);
    ~Mesh();
    Mesh(const Mesh&)            = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept = delete;

    void Draw(VkCommandBuffer commandBuffer, glm::mat4 model = glm::mat4(1.0f), glm::mat3 normal = glm::mat3(1.0f)) const;
    void DrawSimple() const;

    static std::vector<uint32_t> makeVecIndex(const uint32_t* array, size_t size);
    static std::vector<Vertex>   makeVecVertex(const float* array, size_t size);
};

#endif