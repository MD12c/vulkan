// #ifndef ORBIT_CAMERA_H
// #define ORBIT_CAMERA_H

// #include "Camera.h"

// class CameraOrbit : public Camera
// {
// private:
//     glm::vec3 focusPoint = glm::vec3(0.0f, 0.0f, 0.0f);

//     float radius = 5.0f;
//     float yaw = 0, pitch = 0;

//     float FOVdeg    = 45.0f;
//     float nearPlane = 0.1f, farPlane = 10000.0f;

//     float speedYaw   = 100.0f;
//     float speedPitch = 100.0f;
//     float speedZoom  = 1.0f;
//     float speedMove  = 10.0f;

//     glm::vec3 calculatePos();

// public:
//     CameraOrbit();
//     void  Inputs(GLFWwindow* window) override;
//     void  updateScreenSize() override;
//     void  onScroll(GLFWwindow* win, double xoffset, double yoffset) override;
//     float getFOV() const override { return FOVdeg; }

//     void setSpeeds(float speedYaw, float speedPitch, float speedZoom);
//     void setProjection(float FOVdeg, float nearPlane, float farPlane);
//     void setPosition(float orbitRadius, float yaw, float pitch);
// };

// #endif
