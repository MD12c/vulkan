#ifndef SCENE_CLASS_H
#define SCENE_CLASS_H

#include <memory>

#include "glm/glm.hpp"
#include "Device.h"

class Model;

struct PushConstantData
{
    glm::vec2 offset;
    alignas(16) glm::vec3 color;
};

class Scene
{
private:
    void loadModels(Device& device);

public:
    std::unique_ptr<Model> model;

    Scene(Device& device);
    ~Scene();
};

#endif