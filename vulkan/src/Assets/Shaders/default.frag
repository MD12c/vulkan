#version 330 core
out vec4 FragColor;

in vec3 color;
in vec2 TexCoord;
uniform sampler2D tex0;
uniform int useTexture;

void main()
{
   if (useTexture == 1)
      FragColor = texture(tex0, TexCoord) * vec4(color, 1.0);
   else
      FragColor = vec4(color, 1.0);
}