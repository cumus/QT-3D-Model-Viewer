#version 330

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 projection;
uniform mat4 mv;

void main()
{
    gl_Position = projection * mv * vec4(aPos, 1.0);
}
