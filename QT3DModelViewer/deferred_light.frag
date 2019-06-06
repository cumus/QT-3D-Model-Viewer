layout (location = 0) out vec4 FragColor;

uniform vec3 lightColor;

void main(void)
{
    FragColor = vec4(lightColor, 1.0);
}
