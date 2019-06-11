//
varying vec3 vert;
varying vec3 vertNormal;
varying mediump vec4 texc;
varying vec3 bitn;
varying vec3 tn;

uniform highp vec3 lightPos;
uniform highp vec3 light_intensity;

uniform float mode;
uniform vec3 flat_diffuse;

uniform sampler2D texture;

void main()
{
   vec3 L = normalize(lightPos - vert);
   vec3 T = normalize(cross(bitn, tn) * L);

   if (mode == 0) // 0 - Diffuse Texture
       gl_FragColor = texture2D(texture, texc.st);
   else if (mode == 1) // 1 - Vertex Position
       gl_FragColor = vec4(vert, 1.0);
   else if (mode == 2) // 2 - Vertex Normal
       gl_FragColor = vec4(vertNormal, 1.0);
   else if (mode == 3) // 3 - Vertex Texture Coord
       gl_FragColor = texc;
   else if (mode == 4) // 4 - Bitangent
       gl_FragColor = vec4(bitn, 1.0);
   else if (mode == 5) // 5 - Tangent
       gl_FragColor = vec4(tn, 1.0);
   else if (mode == 6) // 6 - Depth
       gl_FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
   else if (mode == 7) // 7 - Linear Depth
   {
       float near = 0.1;
       float far = 100.0;
       float z = gl_FragCoord.z * 2.0 - 1.0;
       float linear_depth = (2.0 * near * far) / (far + near - z * (far - near));
       gl_FragColor = vec4(vec3(1.0 - (linear_depth / far)), 1.0);
   }
   else if (mode == 8) // 8 - ??
       gl_FragColor = vec4(texture2D(texture, texc.st) * T, 1.0);
   else if (mode == -1)
       gl_FragColor = vec4(flat_diffuse, 1.0);
}














