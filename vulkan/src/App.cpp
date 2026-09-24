#include "App.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "Graphics/Cameras/Camera.h"

App::App()
    : window(width, height, name),
      device(window),
      renderer(device, window),
      scene(device)
{
    scene.loadCamera(device, window);
    scene.loadModels(renderer);
    scene.tex = renderer.textureManager.loadTexture(device, "Assets/Models/crow/diffuse.png");
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
