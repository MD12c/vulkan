#include "Scene.h"

#include <iostream>

#include "Model\Model.h"
#include "Cameras/Camera.h"
#include "Cameras/Fly.h"

Scene::Scene(Device& device, Window& window)
{
    {
        model = std::make_unique<Model>(device, "Assets/Models/crow/scene.gltf");
    }

    {
        const float FOV       = 90.0f;
        const float nearPlane = 0.1f;
        const float farPlane  = 400.0f;
        camera                = std::make_unique<CameraFly>(device, window, FOV, nearPlane, farPlane);
    }
}

Scene::~Scene()
{
}
