attribute vec3 aPosition;
attribute vec3 aColor;

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;

void main() {
  vec4 vPosition;

  /* First model transforms */
  vPosition = uModelMatrix* vec4(aPosition.x, aPosition.y, aPosition.z, 1);
  vPosition = uViewMatrix* vPosition;
  gl_Position = uProjMatrix*vPosition;
  gl_FrontColor = vec4(aColor.r, aColor.g, aColor.b, 1.0);
}
