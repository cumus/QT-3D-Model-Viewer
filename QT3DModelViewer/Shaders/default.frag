//
varying vec3 vert;
varying vec3 vertNormal;
varying mediump vec4 texc;
varying vec3 bitn;
varying vec3 tn;

uniform highp vec3 lightPos;
uniform highp vec3 light_intensity;
uniform float mode;
uniform sampler2D texture;

void main()
{
   vec3 L = normalize(lightPos - vert);
   vec3 T = normalize(cross(bitn, tn) * L);

   if (mode == 0)
       gl_FragColor = texture2D(texture, texc.st);
   else if (mode == 1)
       gl_FragColor = vec4(bitn, 1.0);
   else if (mode == 2)
       gl_FragColor = vec4(tn, 1.0);
   else
       gl_FragColor = vec4(texture2D(texture, texc.st) * T, 1.0);
}
