
#ifndef SHADERS_H
#define SHADERS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef int bool;

typedef struct Shader {
  unsigned int ID;

  void (*use) (struct Shader *);
  void (*loadShader) (struct Shader *, char*, char*);

  void (*setInt) (struct Shader *, char*, int);
  void (*setBool) (struct Shader *, char*, bool);
  void (*setFloat) (struct Shader *, char*, float);
} Shader;

void use(Shader *shader);
void loadShader(Shader *shader, char* vertPath, char* fragPath);

void setInt(Shader *shader, char* key, int value);
void setBool(Shader *shader, char* key, bool value);
void setFloat(Shader *shader, char* key, float value);

Shader *getShaderObject();

#endif
