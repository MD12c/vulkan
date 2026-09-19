#version 460 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;

layout(push_constant) uniform Push
{
    vec2 offset;
    vec3 color;
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
    gl_Position = cameraData.proj * cameraData.view * vec4(aPos + push.offset, 0.0f, 1.0f);
}