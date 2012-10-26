uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform vec3 uColor;
uniform vec3 uLight;

attribute vec3 aPosition;
//add a normal attribute
attribute vec3 aNormal;

varying vec3 vColor;

void main() {
  vec4 vPosition;
 
  /* First model transforms */
  vPosition = uModelMatrix* vec4(aPosition.x, aPosition.y, aPosition.z, 1);
  vPosition = uViewMatrix* vPosition;
  gl_Position = uProjMatrix*vPosition;
  //add code to use normal to compute color per vertex
  vec3 L = uLight - aPosition;
  float dist = length(L);
  L = normalize(L);
  float dotProd = max(0, dot(aNormal, L));
  vec3 litColor;
  float ambient = 0.2;
  litColor.r = uColor.r*ambient + uColor.r*dotProd;
  litColor.g = uColor.g*ambient + uColor.g*dotProd;
  litColor.b = uColor.b*ambient + uColor.b*dotProd;
  gl_FrontColor = vec4(litColor.r, litColor.g, litColor.b, 1.0);
  vColor = litColor;
}
