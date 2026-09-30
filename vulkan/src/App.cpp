#include "App.h"
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "Graphics/Cameras/Camera.h"

App::App()
    : window(width, height, name),
      device(window),
      renderer(device, window),
      scene()
{
    scene.loadCamera(device, window, renderer);
    scene.loadModels(renderer);
    scene.loadLights(renderer);
}

App::~App()
{
    vkDeviceWaitIdle(device.device());
}

void App::Update()
{
    window.updateFPS();

    timeCrnt = glfwGetTime();
    timeDiff = timeCrnt - timePrev;
    if (timeDiff >= 1.0 / 60.0)
    {
        timePrev = timeCrnt;
        scene.camera->Inputs();
        if (glfwGetKey(window.getWindow(), GLFW_KEY_B) == GLFW_PRESS)
        {
            scene.directionLights[0].setPosition(scene.camera->Position);
            scene.directionLights[0].setDirection(scene.camera->Orientation);
        }
    }
}

void App::Render()
{
    window.StartFrame();

    renderer.drawFrame(scene);

    window.EndFrame();
}

void App::run()
{
    while (!window.ShouldClose())
    {
        Update();
        Render();
    }
}
