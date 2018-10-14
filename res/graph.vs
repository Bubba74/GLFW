#version 330 core

layout (location = 0) in float rel_height;

uniform int length;
uniform float coordx;
uniform float coordy;
uniform float coordz;
uniform float width;
uniform float height;

uniform int index;

out vec4 color;

void main(){
  float x = coordx + (gl_VertexID+0.0)/length/2.0;
  float y = coordy + rel_height*height;
  float z = coordz;

  gl_Position.x = x;
  gl_Position.y = y;
  gl_Position.z = 0;
  gl_Position.w = 1;

  if (abs(gl_VertexID - index)<3)
    color = vec4(0,1,0,1);
  else
    color = vec4(1,0,0,1);

}
