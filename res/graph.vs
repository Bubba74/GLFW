#version 330 core

layout (location = 0) in float rel_height;

uniform int length;
uniform float xpos, ypos, zpos, width, height;

void main(){
  float x = xpos+(gl_VertexID/width*length);
  float y = ypos + rel_height*height;
  float z = zpos;
  gl_Position = vec4(x, y, z, 1);
}
