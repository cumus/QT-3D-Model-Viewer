#version 330 core
#extension GL_NV_shadow_samplers_cube : enable

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    FragColor = textureCube(skybox, TexCoords);
}
