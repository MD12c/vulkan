#include "Scene.h"
#include <memory>

#include "Cameras/Fly.h"
#include "Renderer.h"
#include "Model/ModelManager.h"

Scene::Scene(Device& device)
    : device(device)
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
void Scene::loadCamera(Device& device, Window& window)
{
    const float FOV       = 45.0f;
    const float nearPlane = 0.1f;
    const float farPlane  = 400.0f;
    camera                = std::make_unique<CameraFly>(device, window, FOV, nearPlane, farPlane);
}
