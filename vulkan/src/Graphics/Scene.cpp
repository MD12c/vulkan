#include "Scene.h"
#include <glm/ext/vector_float3.hpp>
#include <memory>

#include "Cameras/Fly.h"
#include "Lighting/DirectionLight.h"
#include "Renderer.h"
#include "Model/ModelManager.h"

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::loadModels(Renderer& renderer)
{
    // renderer.modelManager.loadModel(models, "Assets/Models/crow/scene.gltf");
    renderer.modelManager.loadModel(models, "Assets/Models/ignore/sponza_palace/scene.gltf");
}
void Scene::loadCamera(Device& device, Window& window, Renderer& renderer)
{
    const float FOV       = 45.0f;
    const float nearPlane = 0.1f;
    const float farPlane  = 400.0f;
    camera                = std::make_unique<CameraFly>(device, window, renderer.descriptorSetsManager, FOV, nearPlane, farPlane);
}

void Scene::loadLights(Renderer& renderer)
{
    const float nearPlane = 0.1f;
    const float farPlane  = 400.0f;

    DirectionLight::DirectionLightInfo createInfo{};
    createInfo.lightColor     = glm::vec3(1.0f, 1.0f, 1.0f);
    createInfo.lightPos       = glm::vec3(0.0f, 0.0f, 0.0f);
    createInfo.lightDirection = glm::vec3(1.0f, 0.0f, 0.0f);
    createInfo.right          = 35.0f;
    createInfo.left           = -35.0f;
    createInfo.top            = 35.0f;
    createInfo.bottom         = -35.0f;
    createInfo.zNear          = nearPlane;
    createInfo.zFar           = farPlane;

    renderer.lightsManager.addDirectionLight(createInfo, directionLights);
}