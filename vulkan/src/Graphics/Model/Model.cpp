#include "Model.h"

#include <cassert>

Model::Model()
{
}

Model::Model(Model&& other) noexcept
    : meshes(std::move(other.meshes)), directory(other.directory), fileType(other.fileType)
{
    other.directory = "Moved";
    other.fileType  = "Moved";
}

void Model::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, MaterialManager& materialManager, Transform transform) const
{
    for (const auto& mesh : meshes)
    {
        PushConst push{};
        push.model  = transform.model;
        push.normal = glm::mat4(transform.normal);
        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConst), &push);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &materialManager.getMat(mesh.materialID).descriptorSet, 0, nullptr);
        mesh.Draw(commandBuffer, transform.model, transform.normal);
    }
}
