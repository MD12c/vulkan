#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTex;
layout(location = 3) in vec3 aTangent;

layout(push_constant) uniform Push
{
    mat4 lightMatrix;
    mat4 model;
}
push;

void main()
{
    gl_Position = push.lightMatrix * push.model * vec4(aPos, 1.0);
}