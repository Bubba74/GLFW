#version 330 core

out vec4 FragColor;

in float y;

uniform vec4 rgba;

void main(){

  FragColor = rgba;

  float dampener = (2*(y+1)-1);
  FragColor *= dampener;
  FragColor.a = 1;

//  float low = -0.3, high = -0.2;
//  if (low < y && y < high)
//  FragColor = vec4(1, 0, 0, 1);

}
