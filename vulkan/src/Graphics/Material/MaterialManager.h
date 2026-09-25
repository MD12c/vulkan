#ifndef MATERIAL_MANAGER_CLASS_H
#define MATERIAL_MANAGER_CLASS_H

#include <vector>
#include <memory>
#include <string>

#include "IMaterial.h"
#include "../Texture/TextureManager.h"
#include "../vkBackend/Device.h"
#include "../vkBackend/DescriptorSetsManager.h"

class MaterialManager
{
public:
    struct PBR_GLTF_LoadInfo
    {
        float       roughness;
        float       metalic;
        std::string albedoMapPath;
        std::string aoMapPath;
        std::string metalicRoughnessMapPath;
        std::string normalMapPath;
        std::string displacementMapPath;
    };

    struct PBR_OBJ_LoadInfo
    {
        float       roughness;
        float       metalic;
        std::string albedoMapPath;
        std::string aoMapPath;
        std::string roughnessMapPath;
        std::string metalicMapPath;
        std::string normalMapPath;
        std::string displacementMapPath;
    };

    struct Specular_LoadInfo
    {
        float       shininess;
        std::string diffuseMapPath;
        std::string specularMapPath;
        std::string normalMapPath;
        std::string displacementMapPath;
    };

private:
    Device&                                 device;
    TextureManager&                         textureManager;
    DescriptorSetsManager&                  descriptorSetsManager;
    std::vector<std::unique_ptr<IMaterial>> materials;

public:
    MaterialManager(Device& device, TextureManager& textureManager, DescriptorSetsManager& descriptorSetsManager);
    ~MaterialManager();
    MaterialManager(const MaterialManager&)             = delete;
    MaterialManager& operator=(const MaterialManager&)  = delete;
    MaterialManager(MaterialManager&& other)            = delete;
    MaterialManager& operator=(MaterialManager&& other) = delete;

    MaterialID LoadMaterialSpecular(Specular_LoadInfo& specularLoadInfo);
    MaterialID LoadMaterialPBRgltf(PBR_GLTF_LoadInfo& pbrLoadInfo);
    MaterialID LoadMaterialPBRobj(PBR_OBJ_LoadInfo& pbrLoadInfo);

    IMaterial& getMat(MaterialID index) { return *materials.at(index); }
};

#endif