uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform vec3 uColor;

attribute vec3 aPosition;
//add a normal attribute

varying vec3 vColor;

void main() {
  vec4 vPosition;
 
  /* First model transforms */
  vPosition = uModelMatrix* vec4(aPosition.x, aPosition.y, aPosition.z, 1);
  vPosition = uViewMatrix* vPosition;
  gl_Position = uProjMatrix*vPosition;
  //add code to use normal to compute color per vertex
  gl_FrontColor = vec4(uColor.r, uColor.g, uColor.b, 1.0);
  vColor = vec3(uColor.r, uColor.g, uColor.b);
}
