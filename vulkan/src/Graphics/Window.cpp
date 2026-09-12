#include "Window.h"
#include "../Globals.h"
#include <stdexcept>

Window::Window(int width, int height, const char* name)
    : width(width), height(height), name(name)
{
    glfwInit();
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(width, height, name, NULL, NULL);
    if (m_window == NULL)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(m_window);

    glfwSetWindowUserPointer(m_window, this);
    auto keyCallback = [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS))
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    };

    glfwSetFramebufferSizeCallback(m_window, framebufferResizedCallBack);
    glfwSetKeyCallback(m_window, keyCallback);

    glfwSwapBuffers(m_window);
    glfwSwapInterval(0);
}

void Window::updateFPS()
{
    // FPS
    static double       prevTime = 0.0;
    static double       crntTime = 0.0;
    static double       timeDiff;
    static unsigned int counter = 0;

    crntTime = glfwGetTime();
    timeDiff = crntTime - prevTime;
    counter++;

    if (timeDiff >= 1.0 / 30.0)
    {
        std::string FPS      = std::to_string((1.0 / timeDiff) * counter);
        std::string ms       = std::to_string((timeDiff / counter) * 1000.0);
        std::string newTitle = std::string(name) + "   " + FPS + " FPS / " + ms + " ms";
        glfwSetWindowTitle(m_window, newTitle.c_str());
        prevTime = crntTime;
        counter  = 0;
    }
}

void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
    if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface");
}

GLFWwindow* Window::getWindow()
{
    return m_window;
}

Window::~Window()
{
    if (m_window)
    {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

void Window::StartFrame()
{
    glfwPollEvents();
}

void Window::EndFrame()
{
    glfwSwapBuffers(m_window);
}

void Window::framebufferResizedCallBack(GLFWwindow* window, int width, int height)
{
    auto ptr = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

    ptr->framebufferResized = true;
    ptr->width              = width;
    ptr->height             = height;
}