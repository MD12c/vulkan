#ifndef MODEL_MANAGER_CLASS_H
#define MODEL_MANAGER_CLASS_H

#include <vector>
#include "../Material/MaterialManager.h"

#include "assimp/scene.h"
#include "Model.h"

class ModelManager
{
private:
    Device&          device;
    MaterialManager& materialManager;

    void processNode(aiNode*, const aiScene*, ModelID modelID, std::vector<Model>& models);
    Mesh processMesh(aiMesh*, const aiScene*, ModelID modelID, std::vector<Model>& models);

public:
    ModelManager(Device& device, MaterialManager& materialManager);
    ~ModelManager();

    void loadModel(std::vector<Model>& models, const std::string& path);
};

#endif