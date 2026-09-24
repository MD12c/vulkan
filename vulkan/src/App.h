#ifndef APP_CLASS_H
#define APP_CLASS_H

#include "Graphics/Renderer.h"
#include "Graphics/Scene.h"

class App
{
private:
    int         width  = 2560;
    int         height = 1440;
    const char* name   = "VK Tutorial";

    double timePrev = 0;
    double timeCrnt = 0;
    double timeDiff;

    Window   window;
    Device   device;
    Renderer renderer;
    Scene    scene;

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