#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);

    // and the diffuse per-fragment color
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec = vec4(texture2D(texture_diffuse1, vec2(TexCoords)).rgb,
                       texture2D(texture_specular1, vec2(TexCoords)).r);
}
