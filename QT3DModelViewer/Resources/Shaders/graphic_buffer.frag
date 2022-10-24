#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform bool use_flat_color;
uniform vec3 flat_color;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{
    gPosition = FragPos;
    gNormal = normalize(Normal);

    if (use_flat_color)
        gAlbedoSpec = vec4(flat_color, 1.0);
    else
        gAlbedoSpec = vec4(texture2D(texture_diffuse1, vec2(TexCoords)).rgb,
                           texture2D(texture_specular1, vec2(TexCoords)).r);
}
