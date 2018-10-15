
#ifndef RUBIKS_H
#define RUBIKS_H

#include <linmath.h>

void vec3_dup(vec3 storage, vec3 values){
  vec3_scale(storage, values, 1);
}//vec3_dup
void vec4_dup(vec4 storage, vec4 values){
  vec4_scale(storage, values, 1);
}//vec4_dup


typedef struct rubiks_tile {

  mat4x4 transform;

  vec3 pos;
  //rgb colors for the faces
  vec4 faces[6];
    //0 - Front
    //1 - right
    //2 - top
    //3 - bottom
    //4 - left
    //5 - back

  unsigned int vao;
  unsigned int ebo_c;

} tile;

void tile__load_vbo(tile *t){
  float dx = 0.5, dy = 0.5, dz = 0.5;

  float vertices[] = {
    //Front frame
    -dx, dy, dz,
     dx, dy, dz,
    -dx,-dy, dz,
     dx,-dy, dz,

    //Back frame
    -dx, dy,-dz,
     dx, dy,-dz,
    -dx,-dy,-dz,
     dx,-dy,-dz
  };

  int indices[] = {
    //Front
    0,1,2, 1,2,3,
    //Right
    1,5,3, 5,3,7,
    //Top
    4,5,0, 5,0,1,
    //Bot
    6,7,2, 7,2,3,
    //Left
    4,0,6, 0,6,2,
    //Back
    4,5,6, 5,6,7
  };

  /* Panel indices
    Front Right   Top   Bot   Left    Back
    0 1   1 5     4 5   6 7   4  0    4 5
    2 3   3 7     0 1   2 3   6  2    6 7
  */
  /* Old method of assignment coordinate values for vertices
    float xDeltas[][] = {
      { -dx,  dx, -dx,  dx}, //Face 0 - Front
      {  dx,  dx,  dx,  dx}, //Face 1 - Right
      { -dx,  dx, -dx,  dx}, //Face 2 - Top
      { -dx,  dx, -dx,  dx}, //Face 3 - Bottom
      { -dx, -dx, -dx, -dx}, //Face 4 - Left
      { -dx,  dx, -dx,  dx}  //Face 5 - Back
    };
    float yDeltas[][] = {
      {  dy,  dy, -dy, -dy}, //Face 0 - Front
      {  dy,  dy, -dy, -dy}, //Face 1 - Right
      {  dy,  dy,  dy,  dy}, //Face 2 - Top
      { -dy, -dy, -dy, -dy}, //Face 3 - Bottom
      {  dy,  dy, -dy, -dy}, //Face 4 - Left
      {  dy,  dy, -dy, -dy}  //Face 5 - Back
    };
    float zDeltas[][] = {
      {  dz,  dz,  dz,  dz}, //Face 0 - Front
      {  dz, -dz,  dz, -dz}, //Face 1 - Right
      { -dz, -dz,  dz,  dz}, //Face 2 - Top
      { -dz, -dz,  dz,  dz}, //Face 3 - Bottom
      { -dz, -dz,  dz,  dz}, //Face 4 - Left
      { -dz, -dz, -dz, -dz}  //Face 5 - Back
    };
  */

  glGenVertexArrays(1, &t->vao);
  glBindVertexArray(t->vao);

  unsigned int vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  unsigned int ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glBindVertexArray(0);

  t->ebo_c = sizeof(indices)/sizeof(float);
}//tile__load_vbo

tile *tile_create(){
  tile *t = malloc(sizeof(tile));
  t->pos[0] = 0, t->pos[1] = 1, t->pos[2] = 0;
  tile__load_vbo(t);
  return t;
}//tile_create

void tile_set_pos(tile *t, vec3 pos){
  vec3_dup(t->pos, pos);
  mat4x4_translate(t->transform, t->pos[0], t->pos[1], t->pos[2]);
}//tile_set_pos

void tile_set_face_color(tile *t, int faceID, vec4 rgba){
  vec3_dup(t->faces[faceID], rgba); //Copy over colors
}//tile_set_face_color

void tile_reset(tile *t){
  mat4x4_translate(t->transform, t->pos[0], t->pos[1], t->pos[2]);
}//tile_reset

void tile_rotate(tile *t, int axis, float rads){
  // mat4x4_iden
}//tile_rotate

void tile_render(tile *t, unsigned int colorLoc){
  glBindVertexArray(t->vao);

  int face;
  for (face=0; face<6; face++){
    glUniform4fv(colorLoc, 1, t->faces[face]);
    glDrawElements(GL_TRIANGLE_STRIP, t->ebo_c, GL_UNSIGNED_INT, 0);
  }
  glBindVertexArray(0);

}//tile_render


typedef struct rubiks_struct {

  //Virtual cube, with rows starting at the top (0) to bottom(2)
  //Columns start at left (0) to right (2)

  vec3 pos;
  mat4x4 transform;

  tile *tiles[27];
  vec4 faceColors[6];

} rubiks;

rubiks *rubiks_create(vec3 pos, vec4 colors[6]){
  rubiks *cube = malloc(sizeof(rubiks));
  vec3_dup(cube->pos, pos);
  mat4x4_translate(cube->transform, cube->pos[0], cube->pos[1], cube->pos[2]);

  int h;
  for (h=0; h<6; h++)
    vec4_dup(cube->faceColors[h], colors[h]);

  //Possible coordinates for tiles, notice 3*3*3 == 27 combos
  float xCoords[] = {-1, 0, 1};
  float yCoords[] = {-1, 0, 1};
  float zCoords[] = {-1, 0, 1};

  int i;
  for (i=0; i<27; i++){
    cube->tiles[i] = tile_create();

    vec3 pos = {xCoords[i%3], yCoords[(i%9)/3], zCoords[i/9]};
    tile_set_pos(cube->tiles[i], pos);

    int j;
    for (j=0; j<6; j++)
      tile_set_face_color(cube->tiles[i], j, cube->faceColors[j]);
  }

  return cube;
}//rubiks_create

void rubiks_render(rubiks *cube, int shaderID){
  //Leave view / perspective matrices to the outer program
  //Set the local (tile transformations) and model (ruibks pos) matrices deliberately
  unsigned int localLoc = glGetUniformLocation(shaderID, "local");
  unsigned int modelLoc = glGetUniformLocation(shaderID, "model");
  unsigned int colorLoc = glGetUniformLocation(shaderID, "rgba");

  glUseProgram(shaderID);
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat *) cube->transform);

  int i;
  for (i=0; i<27; i++){
    //Set the local matrix
    glUniformMatrix4fv(localLoc, 1, GL_FALSE, (const GLfloat *) cube->tiles[i]->transform);
    tile_render(cube->tiles[i], colorLoc);
  }

}//rubiks_render


#endif
