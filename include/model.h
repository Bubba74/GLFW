
#ifndef MODEL_H
#define MODEL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct mesh {
  unsigned int VAO, VBO, EBO;
  int nVertices, nIndices;
  int matIndex;
};

struct model {
  char *name;
  const struct aiScene *aiScene;

  struct mesh *meshes;
  int nMeshes;

  unsigned int *materials;
  int nMaterials;

  char **textureNames;
  int nTextures;
  unsigned int *textureIDs;

};

struct model *model_new(char *);
void model_draw(struct model *);
void model_activate_material(struct model *, int);
void model_free(struct model *);

#endif
