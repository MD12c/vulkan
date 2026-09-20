#ifndef MODEL_CLASS_H
#define MODEL_CLASS_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "..\vkBackend\Device.h"
#include "Mesh.h"
#include "Transform.h"

class Model
{
public:
    struct PushConst
    {
        glm::mat4 model;
        glm::mat4 normal;
    };

private:
    Device& device;

    std::vector<Mesh> meshes;

    void loadModel(const std::string& path);
    void processNode(aiNode*, const aiScene*);
    Mesh processMesh(aiMesh*, const aiScene*);

public:
    std::string directory;
    std::string fileType;

public:
    Model(Device& device, const std::string& path);
    ~Model()                       = default;
    Model(const Model&)            = delete;
    Model& operator=(const Model&) = delete;

    void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Transform transform) const;

    // void setMeshMetalicRoughness(int meshIndex, float metalic, float roughness);
    // void setCustomMaterial(MaterialID materialID);
};

#endif