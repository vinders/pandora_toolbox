#version 450

layout (vertices = 3) out;

void HSMain() {
  gl_TessLevelOuter[0] = 3.0;
  gl_TessLevelOuter[1] = 3.0;
  gl_TessLevelOuter[2] = 3.0;
  gl_TessLevelInner[0] = 3.0;
}
