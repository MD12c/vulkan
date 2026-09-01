#ifndef SETUP
#define SETUP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

class Window
{
private:
    GLFWwindow* m_window = nullptr;
    int         width;
    int         height;
    std::string name;

public:
    Window(int width, int height, std::string windowName);
    void StartFrame();
    void EndFrame();
    bool ShouldClose() { return glfwWindowShouldClose(m_window); };

    void updateFPS();

    GLFWwindow* getWindow();
    ~Window();
};

#endif
