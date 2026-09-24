#include "Fly.h"

#include "../Window.h"

CameraFly::CameraFly(Device& device, Window& window, float FOVdeg, float nearPlane, float farPlane)
    : Camera(device, window),
      FOVdeg(FOVdeg),
      nearPlane(nearPlane),
      farPlane(farPlane)
{
    view = glm::lookAt(Position, Position + Orientation, Up);
    updateScreenSize();
}

void CameraFly::updateScreenSize()
{
    proj = glm::perspective(glm::radians(FOVdeg), (float)window.getExtent().width / (float)window.getExtent().height, nearPlane, farPlane);
}

void CameraFly::Inputs()
{
    GLFWwindow* winCache = window.getWindow();
    // Keyboard
    if (glfwGetKey(winCache, GLFW_KEY_W) == GLFW_PRESS)
        Position += speed * Orientation;
    if (glfwGetKey(winCache, GLFW_KEY_A) == GLFW_PRESS)
        Position += speed * -glm::normalize(glm::cross(Orientation, Up));
    if (glfwGetKey(winCache, GLFW_KEY_S) == GLFW_PRESS)
        Position += speed * -Orientation;
    if (glfwGetKey(winCache, GLFW_KEY_D) == GLFW_PRESS)
        Position += speed * glm::normalize(glm::cross(Orientation, Up));
    if (glfwGetKey(winCache, GLFW_KEY_SPACE) == GLFW_PRESS)
        Position += speed * -Up;
    if (glfwGetKey(winCache, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        Position += speed * Up;
    if (glfwGetKey(winCache, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        speed = 0.4f;
    else
        speed = 0.1f;

    // Mouse
    static bool firstClick = true;
    if (glfwGetMouseButton(winCache, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glfwSetInputMode(winCache, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        // Prevents rotate spike
        if (firstClick)
        {
            glfwSetCursorPos(winCache, (window.getExtent().width / 2), (window.getExtent().height / 2));
            firstClick = false;
        }

        double mouseX;
        double mouseY;
        glfwGetCursorPos(winCache, &mouseX, &mouseY);

        // Shifts the cursor coord to be in the middle of the screen and normalizes them
        float rotY = sensitivity * (float)(mouseY - (window.getExtent().height / 2)) / window.getExtent().height;
        float rotX = sensitivity * (float)(mouseX - (window.getExtent().width / 2)) / window.getExtent().width;

        // Gets new rotated orientation rotate -> mat4, degree, axis
        glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(rotY), glm::normalize(glm::cross(Orientation, Up)));

        // Checks if newOrientation is not too high angle -> vec3, vec3 -> angle between in rad
        if (!((glm::angle(newOrientation, Up) <= glm::radians(5.0f)) || (glm::angle(newOrientation, -Up) <= glm::radians(5.0f))))
            Orientation = newOrientation;

        // Rotates orientation left/right
        Orientation = glm::rotate(Orientation, glm::radians(-rotX), Up);

        // Locks the cursor to the middle of the screen
        glfwSetCursorPos(winCache, (window.getExtent().width / 2), (window.getExtent().height / 2));
    }
    else if (glfwGetMouseButton(winCache, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        glfwSetInputMode(winCache, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstClick = true;
    }
    view = glm::lookAt(Position, Position + Orientation, Up);
}

void CameraFly::onScroll(GLFWwindow* win, double xoffset, double yoffset)
{
    FOVdeg += (float)yoffset * 5;
    FOVdeg = glm::clamp(FOVdeg, 0.0f, 179.0f);
    proj   = glm::perspective(glm::radians(FOVdeg), (float)window.getExtent().width / (float)window.getExtent().height, nearPlane, farPlane);
}