#version 330 core

layout (location = 0) in vec3 pos;

uniform mat4 local;
uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;

out float y;

void main(){
  y = pos.y;
  gl_Position = vec4(pos, 1);
  gl_Position = perspective * view * model * local * gl_Position;
}
