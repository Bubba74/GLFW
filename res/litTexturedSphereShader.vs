#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;

out vec2 TexCoord;
out float brightness;

uniform mat4 local;
uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;

uniform vec3 light_pos;

void main(){
  vec4 normal = normalize(local * vec4(pos, 0));

  vec4 vert_pos = local * vec4(pos, 1);
  vec4 light_dir = normalize(  vec4(light_pos,1) - vert_pos  );

  float dot_product = dot(normal, light_dir);
  if (dot_product < 0)
    brightness = 0;
  else
    brightness = dot_product;

  gl_Position = vec4(pos, 1);
  gl_Position = perspective * view * model * local * gl_Position;


  TexCoord = tex;
}
