attribute vec3 aPosition;
attribute vec2 aTexCoord;

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;

varying vec3 vColor;
varying vec2 vTexCoord;

void main() {
  vec4 vPosition;

  /* First model transforms */
  vPosition = uModelMatrix* vec4(aPosition.x, aPosition.y, aPosition.z, 1);
  vPosition = uViewMatrix* vPosition;
  gl_Position = uProjMatrix*vPosition;
  //gl_Position = vec4(0, 0, 0, 1);

  vColor = vec3(0.56, 0.3, 0.1);
  vTexCoord = aTexCoord;
}
