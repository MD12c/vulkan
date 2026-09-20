#ifndef SCENE_CLASS_H
#define SCENE_CLASS_H

#include <memory>
#include <array>

#include "glm/glm.hpp"
#include "vkBackend\Device.h"
#include "Window.h"

class Model;
class Camera;

struct PushConstantData
{
    glm::vec2 offset;
    alignas(16) glm::vec3 color;
};

class Scene
{
private:
public:
    std::unique_ptr<Model>  model;
    std::unique_ptr<Camera> camera;

    Scene(Device& device, Window& window);
    ~Scene();

    void loadModels(Device& device);
    void loadCamera(Device& device, Window& window);
};

#endif