//
attribute vec4 vertex;
attribute vec3 normal;
attribute mediump vec4 texCoord;
attribute vec3 tangent;
attribute vec3 bitangent;

varying vec3 vert;
varying vec3 vertNormal;
varying mediump vec4 texc;
varying vec3 bitn;
varying vec3 tn;

uniform mat4 projMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

void main()
{
    texc = texCoord;
    vert = vertex.xyz;
    vertNormal = normalMatrix * normal;


    bitn = bitangent;
    tn = tangent;

    gl_Position = projMatrix * mvMatrix * vertex;
}
