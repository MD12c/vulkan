#include "main.h"
#include "Globals.h"
#include "Graphics/Window.h"

int main()
{
    Window window(1920, 1080, "VK Tutorial");

    while (!window.ShouldClose())
    {
        window.StartFrame();
        window.updateFPS();

        window.EndFrame();
    }

    return 0;
}
