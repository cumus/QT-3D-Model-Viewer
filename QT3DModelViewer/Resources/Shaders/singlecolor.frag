//
varying vec4 texc;

uniform vec3 flat_color;
uniform float alpha;

void main(void)
{
    gl_FragColor = vec4(flat_color, alpha);
}
