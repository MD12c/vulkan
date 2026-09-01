#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec3 color;
out vec2 TexCoord;

uniform vec3 Color;
uniform mat4 translated;
uniform mat4 projection;

void main(){
	gl_Position = projection * translated * vec4(aPos, 1.0f);
	color = Color;
	TexCoord = aTexCoord;
}