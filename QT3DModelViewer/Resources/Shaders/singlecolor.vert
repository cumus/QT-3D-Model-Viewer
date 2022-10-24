//
attribute vec3 aPos;
attribute vec4 texCoord;

varying vec4 texc;

uniform mat4 modelview;
uniform mat4 projection;

void main()
{
    texc = texCoord;
    gl_Position = projection * modelview * vec4(aPos, 1.0f);
}
