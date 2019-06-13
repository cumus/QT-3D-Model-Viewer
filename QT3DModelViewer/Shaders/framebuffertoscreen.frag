//
varying vec2 texc;
uniform sampler2D screenTexture;

void main(void)
{
    vec3 col = texture2D(screenTexture, texc).rgb;
    gl_FragColor = vec4(0, col.y, col.z, 1.0);
}
