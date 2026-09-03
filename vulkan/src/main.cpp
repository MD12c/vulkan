#include "main.h"
#include "Globals.h"
#include "App.h"

int main()
{
    try
    {
        App app;
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}
