
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

  GLint inputFormat = GL_RGB;
  if (nrChannels == 4){
    inputFormat = GL_RGBA;
  }
  //               texture     mipmap, xoff, yoff, W, H, format,     type,             pixels
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, inputFormat, GL_UNSIGNED_BYTE, data);
}//updateTexture

void storeTexture(unsigned int texID, int width, int height, int nrChannels, unsigned char *data) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texID);

  GLint inputFormat = GL_RGB;
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

#include <time.h>
#include <grabImage.h>

unsigned int createVideoTexture(char *videoPath, VideoCapture **video){
  printf("Loading video: %s\n", videoPath);
  *video = openVideo(videoPath);
  Mat *first = grabFrame(*video);
  printf("Frame: (%dx%dx%d)\n", matCols(first), matRows(first), matChannels(first));

  unsigned int texture = genTexture();
  storeTexture(texture, matCols(first), matRows(first), matChannels(first), matData(first));
  // releaseMat(first);

  return texture;
} //createVideoTexture

double frameTime = 0, updateTime = 0;
void printVideo(){
  printf("Loading Frame: %.5f\tTransfer to GPU: %.5f\n", frameTime, updateTime);
}//printVideo

void updateTextureWithMat(unsigned int texID, Mat** frame) {
  struct timespec tstart={0,0}, tend={0,0};
  clock_gettime(CLOCK_MONOTONIC, &tstart);
  updateTexture(texID, matCols(*frame), matRows(*frame), matChannels(*frame), matData(*frame));
  clock_gettime(CLOCK_MONOTONIC, &tend);

  updateTime += ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) -
  ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);

  releaseMat(*frame);
  *frame = 0;
}//updateTextureWithMat

void loadVideoTexture(Mat **mat, VideoCapture *video) {
  struct timespec tstart={0,0}, tend={0,0};
  clock_gettime(CLOCK_MONOTONIC, &tstart);
  *mat = grabFrame(video);
  clock_gettime(CLOCK_MONOTONIC, &tend);

  frameTime += ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) -
  ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
}//loadVideoTexture

void loadVideoTextureStruct(MatVideoStruct *obj) {
  loadVideoTexture(&obj->mat, obj->video);
}

void updateVideoTexture(unsigned int texID, VideoCapture *video){
  Mat *frame;
  //(Eventually on a separate thread, ) grab a new frame from the video
  //  and load it into the mat's ->data member
  loadVideoTexture(&frame, video);
  //On the main (graphics) thread, transfer the frame from CPU memory to GPU
  updateTextureWithMat(texID, &frame);
}//updateVideoTexture


#endif
