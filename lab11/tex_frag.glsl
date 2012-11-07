uniform sampler2D uTexUnit;

varying vec2 vTexCoord;
varying vec3 vColor;

void main(void) {
  vec4 texColor0 = vec4(vColor, 1.0);
  vec4 texColor1 = texture2D(uTexUnit, vTexCoord);

  float thresh = 0.5;
  if (texColor1.b > thresh)
  {
     texColor1.g = texColor1.b;
     texColor1.b = 0.0;
  }

  gl_FragColor = vec4(vTexCoord.s, vTexCoord.t, 0.0, 1.0);
  gl_FragColor = vec4(texColor1.rgb, 1.0);
}

