#include "Mesh.h"

#include <cstddef>

Mesh::Mesh(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, MaterialID materialID)
    : device(device), vertices(vertices), indices(indices), materialID(materialID)  // sphere(computeBoundingSphere(vertices))
{
    VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * static_cast<uint32_t>(vertices.size());
    VkDeviceSize indexBufferSize  = sizeof(indices[0]) * static_cast<uint32_t>(indices.size());

    AllocatedBuffer tempVertexBuffer;
    AllocatedBuffer tempIndexBuffer;

    {  // no need to auto flush/send since COHERENT_BIT makes it automatically}
        VmaAllocationCreateInfo allocInfo{};
        allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        allocInfo.usage         = VMA_MEMORY_USAGE_AUTO;
        allocInfo.flags         = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        VkBufferUsageFlags vertexBufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VkBufferUsageFlags indexBufferUsage  = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        // Note Host -> CPU, Device -> GPU
        device.createBuffer(vertexBufferSize, vertexBufferUsage, allocInfo, tempVertexBuffer);
        device.createBuffer(indexBufferSize, indexBufferUsage, allocInfo, tempIndexBuffer);

        {
            void* data;
            vmaMapMemory(device.getVMA(), tempVertexBuffer.allocation, &data);
            memcpy(data, vertices.data(), static_cast<size_t>(vertexBufferSize));
            vmaUnmapMemory(device.getVMA(), tempVertexBuffer.allocation);
        }

        {
            void* data;
            vmaMapMemory(device.getVMA(), tempIndexBuffer.allocation, &data);
            memcpy(data, indices.data(), static_cast<size_t>(indexBufferSize));
            vmaUnmapMemory(device.getVMA(), tempIndexBuffer.allocation);
        }
    }

    {
        VmaAllocationCreateInfo allocInfo{};
        allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        allocInfo.usage         = VMA_MEMORY_USAGE_AUTO;

        VkBufferUsageFlags vertexBufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        VkBufferUsageFlags indexBufferUsage  = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        device.createBuffer(vertexBufferSize, vertexBufferUsage, allocInfo, vertexBuffer);
        device.createBuffer(indexBufferSize, indexBufferUsage, allocInfo, indexBuffer);

        device.copyBuffer(tempVertexBuffer.buffer, vertexBuffer.buffer, vertexBufferSize);
        device.copyBuffer(tempIndexBuffer.buffer, indexBuffer.buffer, indexBufferSize);
    }

    vmaDestroyBuffer(device.getVMA(), tempVertexBuffer.buffer, tempVertexBuffer.allocation);
    vmaDestroyBuffer(device.getVMA(), tempIndexBuffer.buffer, tempIndexBuffer.allocation);
}

Mesh::Mesh(Mesh&& other) noexcept
    : device(other.device), vertices(std::move(other.vertices)), indices(std::move(other.indices))
{
    vertexBuffer = other.vertexBuffer;
    indexBuffer  = other.indexBuffer;
    materialID   = other.materialID;

    other.vertexBuffer.buffer     = VK_NULL_HANDLE;
    other.vertexBuffer.allocation = VK_NULL_HANDLE;
    other.indexBuffer.buffer      = VK_NULL_HANDLE;
    other.indexBuffer.allocation  = VK_NULL_HANDLE;
}

Mesh::~Mesh()
{
    vmaDestroyBuffer(device.getVMA(), vertexBuffer.buffer, vertexBuffer.allocation);
    vmaDestroyBuffer(device.getVMA(), indexBuffer.buffer, indexBuffer.allocation);
}

void Mesh::Draw(VkCommandBuffer commandBuffer, glm::mat4 model, glm::mat3 normal) const
{
    VkDeviceSize pOffsets = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, &pOffsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
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
        { 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent) },
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
