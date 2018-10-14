#version 330 core

layout (location = 0) in vec3 pos;

uniform mat4 local;
uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;

out float x;
out float y;
out float z;


void main(){
  x = pos.x;
  y = pos.y;
  z = pos.z;

  gl_Position = vec4(pos, 1);
  gl_Position = perspective * view * model * local * gl_Position;
}
