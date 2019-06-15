#version 330

layout (location = 0) in vec4 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in mediump vec4 texCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out vec3 vert;
out vec3 vertNormal;
out mediump vec4 texc;
out vec3 bitn;
out vec3 tn;

out vec3 Normal;
out vec3 Position;

uniform mat4 projMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

uniform mat3 modelInvTranMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;

void main()
{
    vert = vertex.xyz;
    vertNormal = normalMatrix * normal;
    texc = texCoord;
    bitn = bitangent;
    tn = tangent;

    mat4 f = transpose(modelMatrix);
    Normal = modelInvTranMatrix * vertNormal;
    Position = vec3(modelMatrix * vec4(vert, 1.0));

    gl_Position = projMatrix * mvMatrix * vertex;
}
