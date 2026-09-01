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
    const char* name;

public:
    Window(int width, int height, const char* name);
    ~Window();
    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;

    void StartFrame();
    void EndFrame();
    bool ShouldClose() { return glfwWindowShouldClose(m_window); }
    void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

    void updateFPS();

    GLFWwindow* getWindow();
    VkExtent2D  getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
};

#endif
