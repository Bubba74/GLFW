#version 330 core

out vec4 FragColor;

in float y;

void main(){
//  float gray = (y+1)/2;

  FragColor = vec4(0, 0, 1, 1);

  float low = -0.3, high = -0.2;

  if (low < y && y < high)
    FragColor = vec4(1, 0, 0, 1);
}
