#include "App.h"

App::App()
    : window(width, height, name), pipeline("default.vert", "default.frag")
{
}

App::~App()
{
}

void App::run()
{
    while (!window.ShouldClose())
    {
        window.StartFrame();
        window.updateFPS();

        window.EndFrame();
    }
}