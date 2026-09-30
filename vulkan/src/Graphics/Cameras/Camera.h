#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include <array>

#include "../vkBackend\DescriptorSetsManager.h"
#include "../vkBackend\Device.h"
#include "../Window.h"
#include "Globals.h"

#include <GLFW/glfw3.h>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

class Camera
{
public:
    glm::mat4 proj  = glm::mat4(1.0f);
    glm::mat4 view  = glm::mat4(1.0f);
    glm::mat4 scale = glm::mat4(1.0f);

    glm::vec3 Position    = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 Up          = glm::vec3(0.0f, 1.0f, 0.0f);

protected:
    Device& device;
    Window& window;

private:
    struct alignas(64) Payload
    {
        glm::mat4 m_proj;
        glm::mat4 m_view;
        glm::vec3 m_camPos;
        Payload(glm::mat4 proj, glm::mat4 view, glm::vec3 camPos)
            : m_proj(proj), m_view(view), m_camPos(camPos) {};
    };

public:
    struct Descriptor
    {
        std::array<AllocatedBuffer, Globals::MAX_FRAMES_IN_FLIGHT> buffers;
        std::array<VkDescriptorSet, Globals::MAX_FRAMES_IN_FLIGHT> descriptorSets;
    };
    Descriptor cameraDescriptors{};

    static constexpr VkDeviceSize payloadSize = sizeof(Payload);

    Camera(Device& device, Window& window, DescriptorSetsManager& descriptorSetManager);
    virtual ~Camera();
    Camera(const Camera&)         = delete;
    void operator=(const Camera&) = delete;
    Camera(Camera&&)              = delete;
    Camera& operator=(Camera&&)   = delete;

    void      updateUniforms(VmaAllocation& allocation) const;
    glm::vec2 screenToWorld(const glm::vec2& pos);

    virtual void  updateScreenSize()                                        = 0;
    virtual void  Inputs()                                                  = 0;
    virtual void  onScroll(GLFWwindow* win, double xoffset, double yoffset) = 0;
    virtual float getFOV() const                                            = 0;

    glm::mat4 getRotationMat();
    glm::mat4 getCameraMat() const { return proj * view * scale; }
};
#endif