// #include "Orbit.h"

// #include "Globals.h"
// #include "../Window.h"

// CameraOrbit::CameraOrbit()
// {
//     Position    = calculatePos() + focusPoint;
//     Orientation = glm::normalize(-Position + focusPoint);
//     view        = glm::lookAt(Position, Position + Orientation, Up);
//     proj        = glm::perspective(glm::radians(FOVdeg), (float)Window::width / (float)Window::height, nearPlane, farPlane);
//     updateScreenSize();
// }

// void CameraOrbit::setProjection(float FOVdeg, float nearPlane, float farPlane)
// {
//     this->FOVdeg    = FOVdeg;
//     this->nearPlane = nearPlane;
//     this->farPlane  = farPlane;
// }
// void CameraOrbit::setPosition(float orbitRadius, float yaw, float pitch)
// {
//     this->radius = orbitRadius;
//     this->yaw    = yaw;
//     this->pitch  = pitch;
// }
// void CameraOrbit::setSpeeds(float speedYaw, float speedPitch, float speedZoom)
// {
//     this->speedYaw   = speedYaw;
//     this->speedPitch = speedPitch;
//     this->speedZoom  = speedZoom;
// }

// glm::vec3 CameraOrbit::calculatePos()
// {
//     float camX = radius * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
//     float camY = radius * sin(glm::radians(pitch));
//     float camZ = radius * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
//     return glm::vec3(camX, camY, camZ);
// }

// void CameraOrbit::Inputs(GLFWwindow* window)
// {
//     static bool firstClick = true;

//     if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
//     {
//         // Rotate
//         glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

//         // Prevents rotate spike
//         if (firstClick)
//         {
//             glfwSetCursorPos(window, (Window::width / 2), (Window::height / 2));
//             firstClick = false;
//         }

//         double mouseX;
//         double mouseY;
//         glfwGetCursorPos(window, &mouseX, &mouseY);

//         // Shifts the cursor coord to be in the middle of the screen and normalizes them
//         float rotX = speedPitch * (float)(mouseX - (Window::width / 2)) / Window::width;
//         float rotY = speedYaw * (float)(mouseY - (Window::height / 2)) / Window::height;

//         yaw -= rotX;
//         pitch += rotY;

//         // Clamp pitch to avoid flipping
//         if (pitch > 89.0f) pitch = 89.0f;
//         if (pitch < -89.0f) pitch = -89.0f;

//         Position = calculatePos() + focusPoint;

//         // Look at origin
//         Orientation = glm::normalize(-Position + focusPoint);

//         // Locks the cursor to the middle of the screen
//         glfwSetCursorPos(window, (Window::width / 2), (Window::height / 2));
//     }
//     else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
//     {
//         // Move
//         glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

//         if (firstClick)
//         {
//             glfwSetCursorPos(window, (Window::width / 2), (Window::height / 2));
//             firstClick = false;
//         }

//         double mouseX;
//         double mouseY;
//         glfwGetCursorPos(window, &mouseX, &mouseY);

//         float moveX = speedMove * (float)(mouseX - (Window::width / 2)) / Window::width;
//         float moveY = speedMove * (float)(mouseY - (Window::height / 2)) / Window::height;

//         glm::vec3 newRight = glm::normalize(glm::cross(Orientation, Up));
//         glm::vec3 newUp    = glm::normalize(glm::cross(newRight, Orientation));
//         focusPoint += -newRight * moveX + newUp * moveY;
//         Position = calculatePos() + focusPoint;

//         glfwSetCursorPos(window, (Window::width / 2), (Window::height / 2));
//     }
//     else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
//     {
//         glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
//         firstClick = true;
//     }

//     view = glm::lookAt(Position, Position + Orientation, Up);
// }

// void CameraOrbit::onScroll(GLFWwindow* win, double xoffset, double yoffset)
// {
//     radius      = glm::clamp(expf(logf(radius) + -(float)yoffset * speedZoom), 0.1f, 1000.0f);
//     Position    = calculatePos() + focusPoint;
//     Orientation = glm::normalize(-Position + focusPoint);
//     view        = glm::lookAt(Position, Position + Orientation, Up);
// }

// void CameraOrbit::updateScreenSize()
// {
//     float aspect = (float)Window::width / (float)Window::height;
//     proj         = glm::perspective(glm::radians(FOVdeg), aspect, nearPlane, farPlane);
// }