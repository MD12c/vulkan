#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTex;
layout(location = 3) in vec3 aTangent;

layout(location = 0) out vec3 oCrntPos;
layout(location = 1) out vec3 oNormal;
layout(location = 2) out vec2 oTex;
layout(location = 3) out vec3 oTangent;

layout(push_constant) uniform Push
{
    mat4 model;
    mat4 normal;
}
push;

layout(set = 0, binding = 0) uniform CameraBuffer
{
    mat4 proj;
    mat4 view;
}
cameraData;

void main()
{
    vec4 crntPos = cameraData.view * push.model * vec4(aPos, 1.0f);
    oTangent     = normalize(vec3(push.model * vec4(aTangent, 0.0f)));
    oTex         = aTex;
    oNormal      = mat3(push.normal) * aNormal;
    oCrntPos     = vec3(crntPos);
    gl_Position  = cameraData.proj * crntPos;
}