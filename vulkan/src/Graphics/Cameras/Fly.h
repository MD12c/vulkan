#ifndef FLY_CAMERA_H
#define FLY_CAMERA_H

#include "Camera.h"

class CameraFly : public Camera
{
private:
    float FOVdeg, nearPlane, farPlane;

    float speed       = 0.1f;
    float sensitivity = 100.0f;

public:
    CameraFly(Device& device, Window& window, float FOVdeg, float nearPlane, float farPlane);
    virtual ~CameraFly() = default;

    void  updateScreenSize() override;
    void  Inputs() override;
    void  onScroll(GLFWwindow* win, double xoffset, double yoffset) override;
    float getFOV() const override { return FOVdeg; }
};

#endif