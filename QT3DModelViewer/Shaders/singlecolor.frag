//
varying vec4 texc;

uniform vec3 flat_color;

void main(void)
{
    gl_FragColor = vec4(flat_color, 1.0);
}
