
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

unsigned int loadTexture(char *texturePath){
// int main(){

  unsigned int width, height, nrChannels;
  unsigned char *data = stbi_load(texturePath, &width, &height, &nrChannels, 0);

  if (!data){
    printf("Error: Failed to load image data from %s\n", texturePath);
    return 0;
  }

  unsigned int texture;
  glGenTextures(1, &texture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  GLint inputFormat = GL_RGB;
  if (nrChannels == 4){
    inputFormat = GL_RGBA;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, inputFormat, width, height, 0, inputFormat, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);

  return texture;
}

unsigned int loadTextureFromString(char *textureData, unsigned int len){
// int main(){

  unsigned int width, height, nrChannels;
  unsigned char *data = stbi_load_from_memory(textureData, len, &width, &height, &nrChannels, 0);

  if (!data){
    printf("Error: Failed to load image data from src of length %d\n", len);
    return 0;
  }

  unsigned int texture;
  glGenTextures(1, &texture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  GLint inputFormat = GL_RGB;
  if (nrChannels == 4){
    inputFormat = GL_RGBA;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, inputFormat, width, height, 0, inputFormat, GL_UNSIGNED_BYTE, data);
  // glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);

  return texture;
}
