
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

unsigned int genTexture(){
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  return texture;
}//getTexture

void updateTexture(unsigned int texID, int width, int height, int nrChannels, unsigned char *data) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texID);

  GLint inputFormat = GL_BGR;
  if (nrChannels == 4){
    inputFormat = GL_RGBA;
  }
  //               texture     mipmap, xoff, yoff, W, H, format,     type,             pixels
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, inputFormat, GL_UNSIGNED_BYTE, data);
}//updateTexture

void storeTexture(unsigned int texID, int width, int height, int nrChannels, unsigned char *data) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texID);

  GLint inputFormat = GL_BGR;
  if (nrChannels == 4){
    inputFormat = GL_RGBA;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, inputFormat, GL_UNSIGNED_BYTE, data);
  // glGenerateMipmap(GL_TEXTURE_2D);
}

unsigned int loadTexture(char *texturePath){
// int main(){

  unsigned int width, height, nrChannels;
  unsigned char *data = stbi_load(texturePath, &width, &height, &nrChannels, 0);

  if (!data){
    printf("Error: Failed to load image data from %s\n", texturePath);
    return 0;
  }

  unsigned int texture = genTexture();

  storeTexture(texture, width, height, nrChannels, data);
  stbi_image_free(data);

  return texture;
}

#define VIDEO_ENABLED
#ifdef VIDEO_ENABLED

#include <grabImage.h>

unsigned int createVideoTexture(char *videoPath, VideoCapture **video){

  *video = openVideo(videoPath);
  Mat *first = grabFrame(*video);
  printf("Frame: (%dx%dx%d)\n", matCols(first), matRows(first), matChannels(first));

  unsigned int texture = genTexture();
  storeTexture(texture, matCols(first), matRows(first), matChannels(first), matData(first));
  // releaseMat(first);

  return texture;
} //createVideoTexture

void updateVideoTexture(unsigned int texID, VideoCapture *video){

  Mat *frame = grabFrame(video);

  updateTexture(texID, matCols(frame), matRows(frame), matChannels(frame), matData(frame));
  releaseMat(frame);

}//updateVideoTexture


#endif
