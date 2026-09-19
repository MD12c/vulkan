#include "Scene.h"

#include <iostream>

#include "Model\Model.h"
#include "Cameras/Camera.h"
#include "Cameras/Fly.h"

Scene::Scene(Device& device, Window& window)
{
    const float FOV       = 90.0f;
    const float nearPlane = 0.1f;
    const float farPlane  = 400.0f;
    loadModels(device);

    camera = std::make_unique<CameraFly>(device, window, FOV, nearPlane, farPlane);
}

Scene::~Scene()
{
}

std::vector<Model::Vertex> subdivide(std::vector<Model::Vertex>& vertices, int it)
{
    std::vector<Model::Vertex> newVertices;
    for (size_t i = 0; i < vertices.size(); i += 3)
    {
        glm::vec2 leftBot  = vertices[i].pos;
        glm::vec2 topCen   = vertices[i + 1].pos;
        glm::vec2 rightBot = vertices[i + 2].pos;

        glm::vec2 newVertex11 = leftBot;
        glm::vec2 newVertex12 = { (topCen.x + leftBot.x) / 2, (topCen.y + leftBot.y) / 2 };
        glm::vec2 newVertex13 = { (rightBot.x + leftBot.x) / 2, (rightBot.y + leftBot.y) / 2 };
        newVertices.push_back({ newVertex11 });
        newVertices.push_back({ newVertex12 });
        newVertices.push_back({ newVertex13 });

        glm::vec2 newVertex21 = newVertex12;
        glm::vec2 newVertex22 = topCen;
        glm::vec2 newVertex23 = { (rightBot.x + topCen.x) / 2, (rightBot.y + topCen.y) / 2 };
        newVertices.push_back({ newVertex21 });
        newVertices.push_back({ newVertex22 });
        newVertices.push_back({ newVertex23 });

        newVertices.push_back({ newVertex13 });
        newVertices.push_back({ newVertex23 });
        newVertices.push_back({ rightBot });
    }
    std::cout << it << "\n";

    if (it < 2)
        return subdivide(newVertices, it + 1);
    else
        return newVertices;
}

void Scene::loadModels(Device& device)
{
    std ::vector<Model ::Vertex> vertices{
        { { -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f } },
        { { 0.0f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
        { { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } }
    };

    model = std::make_unique<Model>(device, vertices);
    // model = std::make_unique<Model>(device, subdivide(vertices, 0));
}
