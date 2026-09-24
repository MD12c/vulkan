#ifndef SCENE_CLASS_H
#define SCENE_CLASS_H

#include <memory>
// #include <array>

// #include "glm/glm.hpp"
#include "Texture\Texture.h"
#include "vkBackend\Device.h"
#include "Window.h"

class Renderer;
class Model;
class Camera;

class Scene
{
private:
    Device& device;

public:
    std::vector<Model>       models;
    std::unique_ptr<Camera>  camera;
    std::shared_ptr<Texture> tex;

    Scene(Device& device);
    ~Scene();

    void loadModels(Renderer& renderer);
    void loadCamera(Device& device, Window& window);
};

#endif