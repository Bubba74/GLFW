
#include <model.h>

struct model *model_new (char *path) {

  struct model *model = malloc(sizeof(struct model));
  model->aiScene = aiImportFile(path, aiProcessPreset_TargetRealtime_MaxQuality);

  if (!model->aiScene) {
    printf("Failed to load scene from file: %s\n", path);
    return;
  }

  const struct aiMesh** meshes = model->aiScene->mMeshes;
  unsigned int nMeshes = model->aiScene->mNumMeshes;
  model->meshes = malloc(nMeshes * sizeof(struct mesh));
  model->nMeshes = nMeshes;

  int meshI;
  for (meshI=0; meshI < nMeshes; meshI++) {
    const struct aiMesh* mesh = model->aiScene->mMeshes[meshI];
    struct mesh *meshData = &model->meshes[meshI];
    meshData->nVertices = mesh->mNumVertices;

    glGenVertexArrays(1, &meshData->VAO);
    // printf("Buffers: %d - %d\n", meshData->VBO, meshData->EBO);
    glGenBuffers(2, &meshData->VBO);
    // printf("Buffers: %d - %d\n", meshData->VBO, meshData->EBO);

    glBindVertexArray(meshData->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, meshData->VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData->EBO);

    // Buffer 3 floats per vertex
      // Note: Hopefully, the vertices array of aiVertex3D respects the compact packing of x,y,z
    glBufferData(GL_ARRAY_BUFFER, 3*sizeof(float)*mesh->mNumVertices, mesh->mVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT ,GL_FALSE, 3*sizeof(float), 0);
    glEnableVertexAttribArray(0);

    int nIndices = 0;
    int faceI;
    for (faceI=0; faceI < mesh->mNumFaces; faceI++) {
      nIndices += mesh->mFaces[faceI].mNumIndices;
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*nIndices, 0, GL_STATIC_DRAW);
    meshData->nIndices = nIndices;

    int offset = 0;
    for (faceI=0; faceI < mesh->mNumFaces; faceI++) {
      glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, sizeof(unsigned int) * mesh->mFaces[faceI].mNumIndices, mesh->mFaces[faceI].mIndices);
      offset += mesh->mFaces[faceI].mNumIndices * sizeof(unsigned int);
    }

    glBindVertexArray(0);
  }

  int vertices = 0;
  printf("Number of meshes: %d\n", model->aiScene->mNumMeshes);
  int i;
  for (i=0; i<model->aiScene->mNumMeshes; i++) {
          // mesh = meshes[i];
          // printf("Mesh %d: Vertices: %d Faces: %d\n", i, mesh->mNumVertices, mesh->mNumFaces);

          vertices += model->aiScene->mMeshes[i]->mNumVertices;
  }
  printf("Total # of vertices: %d\n", vertices);

  return model;
} // model_new

void model_draw(struct model *model) {

  static int print = 0;
  struct mesh *mesh;

  int vertices_drawn = 0;

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  int meshI;
  for (meshI=0; meshI<model->nMeshes; meshI++) {
    mesh = &model->meshes[meshI];

    glBindVertexArray(mesh->VAO);
    if (!print)
      printf("Rendering %d indices\n", mesh->nIndices);
    glDrawElements(GL_TRIANGLES, mesh->nIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    vertices_drawn += mesh->nVertices;
  }
  // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


  if (!print)
    printf("Rendered %d vertices\n", vertices_drawn);
  print = 1;

} // model_draw

void model_free(struct model *model) {
  aiReleaseImport(model->aiScene);
  free(model);
} // model_free
