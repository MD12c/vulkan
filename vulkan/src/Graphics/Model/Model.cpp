#include "Model.h"

#include <cassert>

Model::Model(Device& device, const std::string& path)
    : device(device)
{
    loadModel(path);
}

void Model::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Transform transform) const
{
    for (const auto& mesh : meshes)
    {
        PushConst push{};
        push.model  = transform.model;
        push.normal = glm::mat4(transform.normal);
        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConst), &push);
        mesh.Draw(commandBuffer, transform.model, transform.normal);
    }
}
