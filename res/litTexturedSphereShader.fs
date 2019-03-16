#version 330 core

in vec2 TexCoord;
in float brightness;

out vec4 FragColor;

uniform sampler2D day;
uniform sampler2D night;

void main(){
  vec4 dayColor = texture(day, TexCoord);
  vec4 nightColor = texture(night, TexCoord);

//  FragColor = brightness * dayColor + 0.4 * (1-brightness)*(1-brightness) * nightColor;
  FragColor = 0.5*dayColor + 0.4*brightness*nightColor;

  FragColor.w = 1;
}
