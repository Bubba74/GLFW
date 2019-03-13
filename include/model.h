
#ifndef MODEL_H
#define MODEL_H

#include <stdio.h>
#include <stdlib.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct mesh {
  unsigned int VAO, VBO, EBO;
  int nVertices, nIndices;
};

struct model {
  char *name;
  const struct aiScene *aiScene;

  struct mesh *meshes;
  int nMeshes;
};

struct model *model_new(char *);
void model_draw(struct model *);
void model_free(struct model *);

#endif
