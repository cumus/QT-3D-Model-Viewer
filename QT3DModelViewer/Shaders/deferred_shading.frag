#version 330

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light {
    vec3 Position;
    vec3 Color;

    float Linear;
    float Quadratic;
    float Radius;
};
uniform Light light;
uniform vec3 viewPos;

void main()
{
    // retrieve data from gbuffer
    vec3 FragPos = texture2D(gPosition, TexCoords).rgb;
    vec3 Normal = texture2D(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture2D(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture2D(gAlbedoSpec, TexCoords).a;

    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);

    // calculate distance between light source and current fragment
    float distance = length(light.Position - FragPos);
    if(distance < light.Radius)
    {
        // diffuse
        vec3 lightDir = normalize(light.Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light.Color;
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = light.Color * spec * Specular;
        // attenuation
        float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;
    }
    FragColor = vec4(lighting, 1.0);
}
