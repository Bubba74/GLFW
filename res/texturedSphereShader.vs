#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;

out vec2 TexCoord;

uniform mat4 local;
uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;

void main(){
  gl_Position = vec4(pos, 1);
  gl_Position = perspective * view * model * local * gl_Position;

  TexCoord = tex;
}
