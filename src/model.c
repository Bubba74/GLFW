
#include <model.h>

unsigned int loadTexture(char *path);

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
    meshData->matIndex  = mesh->mMaterialIndex;

    glGenVertexArrays(1, &meshData->VAO);
    // printf("Buffers: %d - %d\n", meshData->VBO, meshData->EBO);
    glGenBuffers(2, &meshData->VBO);
    // printf("Buffers: %d - %d\n", meshData->VBO, meshData->EBO);

    glBindVertexArray(meshData->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, meshData->VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData->EBO);

    // Buffer 3 floats per vertex
    // Note: Hopefully, the vertices array of aiVertex3D respects the compact packing of x,y,z
    int size_verts = sizeof(float)*mesh->mNumVertices;
    glBufferData(GL_ARRAY_BUFFER, 5*size_verts, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,            3*size_verts, mesh->mVertices);
    glBufferSubData(GL_ARRAY_BUFFER, 3*size_verts, 2*size_verts, mesh->mTextureCoords[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)(3*size_verts));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    printf("Mesh %d has %d vertices and %d texture coords\n", meshI, mesh->mNumVertices, mesh->mNumUVComponents[0]);

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

  // Loading Materials
  int matI;
  model->nMaterials = model->aiScene->mNumMaterials;
  // Really only store one texture id per material
  model->materials = malloc(model->nMaterials*sizeof(unsigned int));
  for (matI = 0; matI < model->nMaterials; matI++)
    model->materials[matI] = -1;

  int pathLen = strlen(path);
  char modelDir[200];
  int  modelDirLen = 0;

  int c;
  for (c = 0; c < pathLen; c++) {
    modelDir[c] = path[c];
    if (path[c] == '/')
      modelDirLen = c+1;
  }

  model->nTextures = 0;
  model->textureNames   = malloc(model->nTextures * sizeof(char *));
  model->textureIDs   = malloc(model->nTextures * sizeof(unsigned int));

  printf("\n\n      Loading %d Materials      \n", model->nMaterials);
  for (matI=0; matI < model->nMaterials; matI++) {
    struct aiMaterial *mat = model->aiScene->mMaterials[matI];
    unsigned int nTextures = aiGetMaterialTextureCount(mat, aiTextureType_DIFFUSE);
    printf("Mat %d/%d has %d textures\n", matI, model->nMaterials, nTextures);

    struct aiString path;
    int textI, flags;
    for (textI=0; textI < nTextures; textI++) {
      if (textI == 1) break; // Only load first texture
      if (AI_SUCCESS == aiGetMaterialTexture(mat, aiTextureType_DIFFUSE, textI, &path, 0,0,0, 0,0,&flags)) {
        // printf("Succeeded loading texture %d!\n", textI);
        // printf("Loading texture: %s\n", path.data);

        int fileStart = 0;   // Get position of last filename in directory path
        for (c=0; path.data[c] != '\0'; c++) {
          if (path.data[c] == '/')
            fileStart = c+1;
        }

        // Copy the texture path (starting at the filename) onto the model dir,
            // (starting after the last forward slash)
        strcpy(modelDir+modelDirLen, path.data+fileStart);

        // printf("Determined texture path: %s\n", modelDir);
        int tI;
        for (tI = 0; tI < model->nTextures; tI++) {
          // If the texture path matches that stored, copy the texture id
              // into the materials texture ID spot
          if (!strcmp(model->textureNames[tI], path.data+fileStart)) {
            printf("Found matching existing texture!  %s  %s\n",
                              model->textureNames[tI], modelDir);
            model->materials[matI] = model->textureIDs[tI];
            break;
          }
        }

        if (tI == model->nTextures) {// Not found
          // printf("Creating new texture\n");
          model->nTextures++;

          // Add on another char* to texture names
          model->textureNames = realloc(model->textureNames, model->nTextures * sizeof(char*));
          // printf("Realloc'd texture names\n");

          // Copy modelDir to texture names
          model->textureNames[model->nTextures-1] = malloc(100);
          strcpy(model->textureNames[model->nTextures-1], path.data+fileStart);

          // Add on another texture ID
          model->textureIDs   = realloc(model->textureIDs, model->nTextures * sizeof(unsigned int));
          // printf("Realloc'd texture IDs\n");

          // printf("Loaded texture!\n");
          model->materials[matI] =
          model->textureIDs[model->nTextures-1] = loadTexture(modelDir);
        }
      } else
        printf("Failed to load texture %d\n", textI);
    }
  }

  int d; printf("----- Loaded %d textures\n", model->nTextures);
  for (d=0; d<model->nTextures; d++)
    printf("    %2d: %d: %s\n", d, model->textureIDs[d], model->textureNames[d]);

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

    model_activate_material(model, mesh->matIndex);

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

void model_activate_material(struct model *model, int matIndex) {
  int textureIndex = model->materials[matIndex];
  if (textureIndex < 0) return;

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureIndex);
} // model_activate_material


void model_free(struct model *model) {
  aiReleaseImport(model->aiScene);
  free(model);
} // model_free
