//
attribute vec2 vertex;
attribute vec2 texCoord;

varying vec2 texc;

void main(void)
{
    texc = texCoord;
    gl_Position = vec4(vertex.x, vertex.y, 0.0, 1.0);
}
