uniform sampler2D uTexUnit;

varying vec2 vTexCoord;
varying vec3 vColor;

void main(void) {
  vec4 texColor0 = vec4(vColor.x, vColor.y, vColor.z, 1);
  vec4 texColor1 = texture2D(uTexUnit, vTexCoord);

  gl_FragColor = vec4(vTexCoord.s, vTexCoord.t, 0, 1);
  gl_FragColor = vec4(texColor1[0], texColor1[1], texColor1[2], 1);
}

