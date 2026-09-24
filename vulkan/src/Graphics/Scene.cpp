#include "Scene.h"
#include <memory>

#include "Model\Model.h"
#include "Cameras/Fly.h"
#include "Renderer.h"

Scene::Scene(Device& device)
    : device(device)
{
}

Scene::~Scene()
{
}

void Scene::loadModels(Renderer& renderer)
{
    model = std::make_unique<Model>(device, "Assets/Models/crow/scene.gltf");
    // model = std::make_unique<Model>(device, "Assets/Models/ignore/sponza_palace/scene.gltf");
}
void Scene::loadCamera(Device& device, Window& window)
{
    const float FOV       = 90.0f;
    const float nearPlane = 0.1f;
    const float farPlane  = 400.0f;
    camera                = std::make_unique<CameraFly>(device, window, FOV, nearPlane, farPlane);
}
