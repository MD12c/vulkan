#ifndef MODEL_TRANSFORM_CLASS_H
#define MODEL_TRANSFORM_CLASS_H

#include "glm/gtc/quaternion.hpp"

class Transform
{
public:
    glm::mat4 model;
    glm::mat3 normal;

    Transform(
        glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::quat rotation    = glm::quat(0.0f, 1.0f, 0.0f, 0.0f),
        glm::vec3 scale       = glm::vec3(1.0f, 1.0f, 1.0f))
    {
        glm::mat4 transMat4 = glm::translate(glm::mat4(1.0f), translation);
        glm::mat4 rotMat4   = glm::mat4_cast(rotation);
        glm::mat4 scaMat4   = glm::scale(glm::mat4(1.0f), scale);

        model  = transMat4 * rotMat4 * scaMat4;
        normal = glm::transpose(glm::inverse(glm::mat3(model)));
    }
};

#endif