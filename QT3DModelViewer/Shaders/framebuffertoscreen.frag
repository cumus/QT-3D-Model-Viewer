//
in vec2 texc;
uniform sampler2D ambient;
uniform sampler2D diffuse;
uniform sampler2D specular;

uniform float mode;

void main(void)
{
    mode = 0;

    if (mode == 0)
        gl_FragColor = vec4(texture2D(ambient, texc).rgb, 1.0);
    else if (mode == 1)
        gl_FragColor = vec4(texture2D(diffuse, texc).rgb, 1.0);
    else if (mode == 2)
        gl_FragColor = vec4(texture2D(specular, texc).rgb, 1.0);
}
