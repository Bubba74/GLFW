#version 330 core

layout (location = 0) in vec2 coords;
layout (location = 1) in vec2 texCoords;

out vec2 TexCoord;

uniform mat4 vp;
uniform mat4 local;

void main(){
  gl_Position = vec4(coords.x, 0, coords.y, 1);
  gl_Position = vp * local * gl_Position;

  TexCoord = texCoords;
  TexCoord.y = 1-TexCoord.y;
}
