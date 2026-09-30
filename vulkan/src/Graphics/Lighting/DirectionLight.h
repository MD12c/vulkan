#ifndef DIRECTION_MAP_CLASS_H
#define DIRECTION_MAP_CLASS_H

#include <vulkan/vulkan_core.h>

#include <cstdint>

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_int4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "ShadowMapDimensions.h"

class DirectionLight
{
private:
    glm::vec3 pos, dir, color;

    glm::mat4 proj = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);

public:
    uint32_t layerIndex;

    inline static bool updateUBOdata = true;

    struct DirLightsUBO
    {
        glm::vec4  direction[ShadowMapDimensions::MAX_DIR_LIGHTS];
        glm::vec4  color[ShadowMapDimensions::MAX_DIR_LIGHTS];
        glm::mat4  shadowMatrix[ShadowMapDimensions::MAX_DIR_LIGHTS];
        glm::ivec4 layerIndex[ShadowMapDimensions::MAX_DIR_LIGHTS];
        uint32_t   numDirLights;
        uint32_t   _pad[3];
    };

    struct DirectionLightInfo
    {
        float     left;
        float     right;
        float     bottom;
        float     top;
        float     zNear;
        float     zFar;
        glm::vec3 lightPos;
        glm::vec3 lightDirection;
        glm::vec3 lightColor;
    };

    DirectionLight(uint32_t layerIndex, DirectionLightInfo createInfo)
        : pos(createInfo.lightPos), dir(createInfo.lightDirection), color(createInfo.lightColor), layerIndex(layerIndex)
    {
        proj = glm::ortho(createInfo.left, createInfo.right, createInfo.bottom, createInfo.top, createInfo.zNear, createInfo.zFar);
        view = glm::lookAt(createInfo.lightPos, createInfo.lightPos + createInfo.lightDirection, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void BeginDepthPass(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, glm::mat4 model) const
    {
        ShadowMapDimensions::Shadow2DPushConst payload(proj, view, model);
        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ShadowMapDimensions::Shadow2DPushConst), &payload);
    }

    void UpdateView(glm::vec3 newPos, glm::vec3 newDirection)
    {
        pos           = newPos;
        dir           = newDirection;
        view          = glm::lookAt(newPos, newPos + newDirection, glm::vec3(0.0f, 1.0f, 0.0f));
        updateUBOdata = true;
    }

    void      setPosition(glm::vec3 newPos) { UpdateView(newPos, dir); }
    void      setDirection(glm::vec3 newDirection) { UpdateView(pos, newDirection); }
    void      setColor(glm::vec3 newColor) { color = newColor; }
    glm::vec3 getPosition() const { return pos; }
    glm::vec3 getDirection() const { return dir; }
    glm::vec3 getColor() const { return color; }
    glm::mat4 getShadowMatrix() const { return proj * view; }
};

#endif