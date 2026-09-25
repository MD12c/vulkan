#ifndef PBR_MATERIAL_CLASS_H
#define PBR_MATERIAL_CLASS_H

#include <memory>

#include "glm/glm.hpp"
#include "IMaterial.h"
#include "../Texture/Texture.h"

class PBRMaterial : public IMaterial
{
public:
    glm::vec3 albedoColor = glm::vec3(0.7f);
    float     roughness;
    float     metalic;

    static constexpr size_t  NUM_TEXTURES        = 5;
    std::shared_ptr<Texture> albedoMap           = nullptr;
    std::shared_ptr<Texture> aoMap               = nullptr;
    std::shared_ptr<Texture> metalicRoughnessMap = nullptr;
    std::shared_ptr<Texture> normalMap           = nullptr;
    std::shared_ptr<Texture> displacementMap     = nullptr;

    PBRMaterial(
        int                      ID,
        float                    roughness,
        float                    metalic,
        std::shared_ptr<Texture> albedoMap,
        std::shared_ptr<Texture> aoMap,
        std::shared_ptr<Texture> metalicRoughnessMap,
        std::shared_ptr<Texture> normalMap,
        std::shared_ptr<Texture> displacementMap)

        : IMaterial(ID),
          roughness(roughness),
          metalic(metalic),
          albedoMap(albedoMap),
          aoMap(aoMap),
          metalicRoughnessMap(metalicRoughnessMap),
          normalMap(normalMap),
          displacementMap(displacementMap)
    {
    }

    void Apply() const override
    {
    }
};

#endif