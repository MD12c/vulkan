#ifndef MODEL_CLASS_H
#define MODEL_CLASS_H

#include <cstdint>
#include <string>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "Mesh.h"
#include "Transform.h"
#include "../Material/MaterialManager.h"
#include "../Lighting/DirectionLight.h"

using ModelID = uint32_t;

class Model
{
public:
    std::vector<Mesh> meshes;

    std::string directory;
    std::string fileType;

    struct PushConstModel
    {
        glm::mat4 model;
        glm::mat4 normal;
    };

    Model();
    ~Model()                       = default;
    Model(const Model&)            = delete;
    Model& operator=(const Model&) = delete;
    Model(Model&& other) noexcept;
    Model& operator=(Model&& other) = delete;

    void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, MaterialManager& materialManager, Transform transform) const;
    void DrawShadow(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, const DirectionLight& dirLigth, Transform transform) const; // TODO make an interface for lights

    // void setMeshMetalicRoughness(int meshIndex, float metalic, float roughness);
    // void setCustomMaterial(MaterialID materialID);
};

#endif