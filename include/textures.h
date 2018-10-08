
#ifndef TEXTURES_H
#define TEXTURES_H

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

unsigned int loadTexture(char *filePath);
unsigned int loadTextureFromString(char *imageData, unsigned int len);

#endif
