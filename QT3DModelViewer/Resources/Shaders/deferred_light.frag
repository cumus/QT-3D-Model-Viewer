#version 330

layout (location = 0) out vec4 gl_Color;

uniform vec3 lightColor;

void main(void)
{
    gl_Color = vec4(lightColor, 1.0);
}
