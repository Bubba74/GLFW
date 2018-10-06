#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D tileTexture;

void main(){
  FragColor = texture(tileTexture, TexCoord);
}
