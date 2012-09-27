attribute vec2 aPosition;
attribute vec3 aColor;

void main() {
  gl_Position = vec4(aPosition.x, aPosition.y, 0, 1);
  gl_FrontColor = vec4(aColor.r, aColor.g, aColor.b, 1.0);
}
