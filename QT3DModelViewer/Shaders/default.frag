//
in vec3 vert;
in vec3 vertNormal;
in mediump vec4 texc;
in vec3 bitn;
in vec3 tn;

in vec3 Normal;
in vec3 Position;
uniform vec3 cameraPos;

uniform float mode;

uniform samplerCube skybox;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{
   vec3 T = normalize(cross(bitn, tn));

   float ratio = 1.00 / 1.52;
   vec3 I = normalize(Position - cameraPos);
   vec3 R = reflect(I, normalize(vertNormal));
   vec3 reflect_color = textureCube(skybox, R).rgb;
   vec3 diffuse = texture2D(texture_diffuse1, texc.st).rgb;

   gl_FragColor = vec4(reflect_color, 1.0);

   /*if (mode == 0) // 0 - Diffuse Texture
       gl_FragColor  = texture2D(texture_diffuse1, texc.st);
   else if (mode == 1) // 1 - Vertex Position
       gl_FragColor  = vec4(vert, 1.0);
   else if (mode == 2) // 2 - Vertex Normal
       gl_FragColor  = vec4(vertNormal, 1.0);
   else if (mode == 3) // 3 - Vertex Texture Coord
       gl_FragColor  = texc;
   else if (mode == 4) // 4 - Bitangent
       gl_FragColor  = vec4(bitn, 1.0);
   else if (mode == 5) // 5 - Tangent
       gl_FragColor  = vec4(tn, 1.0);
   else if (mode == 6) // 6 - Depth
       gl_FragColor  = vec4(vec3(gl_FragCoord.z), 1.0);
   else if (mode == 7) // 7 - Linear Depth
   {
       float near = 0.1;
       float far = 100.0;
       float z = gl_FragCoord.z * 2.0 - 1.0;
       float linear_depth = (2.0 * near * far) / (far + near - z * (far - near));
       gl_FragColor  = vec4(vec3(1.0 - (linear_depth / far)), 1.0);
   }
   else if (mode == 8) // 8 - ??
       gl_FragColor  = vec4(vec3(texture2D(texture_diffuse1, texc.st)) * dot(I,T), 1.0);
   else if (mode == 9)
       gl_FragColor = vec4(texture2D(texture_diffuse1, vec2(texc)).rgb, texture2D(texture_specular1, vec2(texc)).r);
   */
}
