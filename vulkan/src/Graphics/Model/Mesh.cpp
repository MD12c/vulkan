#include "Mesh.h"

#include <cstddef>

Mesh::Mesh(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    : device(device), vertices(vertices), indices(indices)  // sphere(computeBoundingSphere(vertices))
{
    // no need to auto flush/send since COHERENT_BIT makes it automatically}
    VkMemoryPropertyFlags properties       = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkDeviceSize          vertexBufferSize = sizeof(vertices[0]) * static_cast<uint32_t>(vertices.size());
    VkDeviceSize          indexBufferSize  = sizeof(indices[0]) * static_cast<uint32_t>(indices.size());

    // Note Host -> CPU, Device -> GPU
    device.createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, properties, vertexBuffer, vertexBufferMemory);
    device.createBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, properties, indexBuffer, indexBufferMemory);

    {
        void* data;
        vkMapMemory(device.device(), vertexBufferMemory, 0, vertexBufferSize, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(vertexBufferSize));
        vkUnmapMemory(device.device(), vertexBufferMemory);
    }

    {
        void* data;
        vkMapMemory(device.device(), indexBufferMemory, 0, indexBufferSize, 0, &data);
        memcpy(data, indices.data(), static_cast<size_t>(indexBufferSize));
        vkUnmapMemory(device.device(), indexBufferMemory);
    }
}

Mesh::Mesh(Mesh&& other) noexcept
    : device(other.device), vertices(std::move(other.vertices)), indices(std::move(other.indices))
{
    vertexBuffer       = other.vertexBuffer;
    vertexBufferMemory = other.vertexBufferMemory;
    indexBuffer        = other.indexBuffer;
    indexBufferMemory  = other.indexBufferMemory;

    other.vertexBuffer       = VK_NULL_HANDLE;
    other.vertexBufferMemory = VK_NULL_HANDLE;
    other.indexBuffer        = VK_NULL_HANDLE;
    other.indexBufferMemory  = VK_NULL_HANDLE;
}

Mesh::~Mesh()
{
    vkDestroyBuffer(device.device(), vertexBuffer, nullptr);
    vkFreeMemory(device.device(), vertexBufferMemory, nullptr);
    vkDestroyBuffer(device.device(), indexBuffer, nullptr);
    vkFreeMemory(device.device(), indexBufferMemory, nullptr);
}

void Mesh::Draw(VkCommandBuffer commandBuffer, glm::mat4 model, glm::mat3 normal) const
{
    VkDeviceSize pOffsets = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &pOffsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}

void Mesh::DrawSimple() const
{  // TODO
}

std::vector<VkVertexInputBindingDescription> Vertex::getBindingDescriptions()
{
    return { { 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX } };
    // Binding, Stride, InputRate
    // VK_VERTEX_INPUT_RATE_VERTEX Advancing stride per vertex
    // VK_VERTEX_INPUT_RATE_INSTANCE Advancing stride per instance
}
std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions()
{
    return {
        { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) },
        { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) },
        { 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texUV) },
        { 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent) }
    };
    // Location, Binding, format, offset
}

std::vector<uint32_t> Mesh::makeVecIndex(const uint32_t* array, size_t size)
{
    std::vector<uint32_t> vec;
    for (size_t i = 0; i < size; i++)
        vec.push_back(array[i]);

    return vec;
}

std::vector<Vertex> Mesh::makeVecVertex(const float* array, size_t size)
{
    std::vector<Vertex> vec;
    for (size_t i = 0; i < size; i += 11)
    {
        Vertex vertex;
        vertex.position = glm::vec3(array[i], array[i + 1], array[i + 2]);
        vertex.normal   = glm::vec3(array[i + 3], array[i + 4], array[i + 5]);
        vertex.texUV    = glm::vec2(array[i + 6], array[i + 7]);
        vertex.tangent  = glm::vec3(array[i + 8], array[i + 9], array[i + 10]);
        vec.push_back(vertex);
    }
    return vec;
}
