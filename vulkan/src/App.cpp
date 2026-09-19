#include "App.h"

#include <iostream>
#include <stdexcept>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "Graphics/Cameras/Camera.h"

App::App()
    : window(width, height, name),
      device(window),
      scene(device, window),
      renderer(device, window)
{
}

App::~App()
{
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
