//
in vec3 vert;
in vec3 vertNormal;
in mediump vec4 texc;
in vec3 bitn;
in vec3 tn;
in vec3 Position;

uniform vec3 cameraPos;
uniform float mode;
uniform float refraction_index;

uniform samplerCube skybox;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{
   vec3 T = normalize(cross(bitn, tn));
   vec3 I = normalize(Position - cameraPos);

   if (mode == 0) // 0 - Diffuse Texture
       gl_FragColor  = vec4(1.0); //texture2D(texture_diffuse1, texc.st);
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
   else if (mode == 8) // 8 - Reflect
       gl_FragColor  = vec4(textureCube(skybox, reflect(I, vertNormal)).rgb, 1.0);
   else if (mode == 9) // 9 - Refract
       gl_FragColor  = vec4(textureCube(skybox, vec3(refract(I, vertNormal, 1.00 / refraction_index))).rgb, 1.0);
}
