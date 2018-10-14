#version 330 core

out vec4 FragColor;

in float x;
in float y;
in float z;

uniform vec4 rgba;

void main(){

  FragColor = rgba;

  float dampenerX = (2*(x+1)-1);
  float dampenerY = (2*(y+1)-1);
  float dampZ = (2*(z+1)-1);


  FragColor.r *= dampenerX;
  FragColor.g *= dampenerY;
  FragColor.b *= dampZ;
  FragColor.a = 1;

//  float low = -0.3, high = -0.2;
//  if (low < y && y < high)
//  FragColor = vec4(1, 0, 0, 1);

}
