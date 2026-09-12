#include "App.h"

#include <iostream>
#include <stdexcept>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

App::App()
    : window(width, height, name),
      device(window),
      scene(device),
      renderer(device, window)
{
}

App::~App()
{
}

void App::Update()
{
    window.updateFPS();
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
