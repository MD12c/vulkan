#ifndef APP_CLASS_H
#define APP_CLASS_H

#include <memory>
#include <vector>

#include "Graphics/Renderer.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Scene.h"

class App
{
private:
    int         width  = 1920;
    int         height = 1080;
    const char* name   = "VK Tutorial";

    Window   window;
    Device   device;
    Scene    scene;
    Renderer renderer;

    void Update();
    void Render();

public:
    App();
    ~App();
    App(const App&)              = delete;
    App& operator=(const App&)   = delete;
    App(const App&&)             = delete;
    App&& operator=(const App&&) = delete;

    void run();
};

#endif