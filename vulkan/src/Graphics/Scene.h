#ifndef SCENE_CLASS_H
#define SCENE_CLASS_H

#include <memory>
#include <vector>
// #include <array>

#include "Lighting/DirectionLight.h"
#include "vkBackend\Device.h"
#include "Window.h"

class Renderer;
class Model;
class Camera;

class Scene
{
public:
    std::vector<Model>          models;
    std::unique_ptr<Camera>     camera;
    std::vector<DirectionLight> directionLights;

    Scene();
    ~Scene();

    void loadModels(Renderer& renderer);
    void loadCamera(Device& device, Window& window, Renderer& renderer);
    void loadLights(Renderer& renderer);
};

#endif