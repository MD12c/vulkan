#ifndef SHADOW_MAP_DIMENSIONS_H
#define SHADOW_MAP_DIMENSIONS_H

#include <cstdint>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/ext/matrix_float4x4.hpp>

namespace ShadowMapDimensions
{
inline constexpr uint32_t SHADOW_MAP_WIDTH  = 2048;
inline constexpr uint32_t SHADOW_MAP_HEIGHT = 2048;

inline constexpr float    CLAMP_COLOR[4]   = { 1.0f, 1.0f, 1.0f, 1.0f };
inline constexpr uint32_t MAX_DIR_LIGHTS   = 8;
inline constexpr uint32_t MAX_SPOT_LIGHTS  = 8;
inline constexpr uint32_t MAX_POINT_LIGHTS = 8;

struct Shadow2DPushConst
{
    glm::mat4 lightMatrix;
    glm::mat4 model;
    Shadow2DPushConst(glm::mat4 proj, glm::mat4 view, glm::mat4 model)
        : model(model) { lightMatrix = proj * view; }
};

};  // namespace ShadowMapDimensions

#endif