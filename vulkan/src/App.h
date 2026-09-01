#ifndef APP_CLASS_H
#define APP_CLASS_H

#include "Graphics/Window.h"
#include "Graphics/Pipeline.h"

class App
{
private:
    int         width  = 1920;
    int         height = 1080;
    const char* name   = "VK Tutorial";
    Window      window;
    Pipeline    pipeline;

public:
    App();
    ~App();

    void run();
};

#endif