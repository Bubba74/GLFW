
#ifndef RUBIKS_H
#define RUBIKS_H

#include <linmath.h>
#include <Shader.h>

void vec3_dup(vec3 storage, vec3 value){
  vec3_scale(storage, value, 1);
}//vec3_dup

typedef struct rubiks_tile {

  mat4 transform;

  vec3 pos;
  //rgb colors for the faces
  vec3 faces[6];
    //0 - top
    //1 - right
    //2 - front
    //3 - back
    //4 - left
    //5 - bottom

} tile;

tile tile_create(){
  tile *t = malloc(sizeof(tile));
  t->pos[0] = 0, t->pos[1] = 1, t->pos[2] = 0;
  return t;
}//tile_create

void tile_set_pos(tile t, vec3 pos){
  vec3_dup(t->pos, pos);
}//tile_set_pos

void tile_set_face_color(tile t, int faceID, vec3 rgb){
  vec3_dup(t->faces[faceID], rgb); //Copy over colors
}//tile_set_face_color


void tile_reset(tile *t){
  mat4x4_translate(t->transform, t->pos[0], t->pos[1], t->pos[2]);
  mat4x4_translate(t->transform, t->pos[0], t->pos[1], t->pos[2]);
}//tile_reset

void tile_rotate(tile *t, int axis, float rads){
  // mat4x4_iden
}


typedef struct rubiks_struct {

  //Virtual cube, with rows starting at the top (0) to bottom(2)
  //Columns start at left (0) to right (2)

  vec3 pos;

  tile *tiles[27];
  vec3 faceColors[6];

} rubiks;

rubiks *rubiks_create(vec3 pos, vec3 colors[6]){
  rubiks *cube = malloc(sizeof(rubiks));
  vec3_dup(cube->pos, pos);

  int h;
  for (h=0; h<6; h++)
    vec3_dup(cube->faceColors[h], colors[h]);

  //Possible coordinates for tiles, notice 3*3*3 == 27 combos
  float xCoords[] = {-1, 0, 1};
  float yCoords[] = {-1, 0, 1};
  float zCoords[] = {-1, 0, 1};

  int i;
  for (i=0; i<27; i++){
    tiles[i] = tile_create();

    vec3 pos = {xCoords[i%3], yCoords[(i%9)/3], zCoords[i/9]};
    tile_set_pos(tiles[i], pos);

    int j;
    for (j=0; j<6; j++)
      tile_set_face_color(tiles[i], j, cube->faceColors[j]);
  }

  return cube;
}//rubiks_create

void rubiks_render(rubiks *cube, Shader *shaderProg){
  //Leave view / perspective matrices to the outer program
  //Set the local (tile transformations) and model (ruibks pos) matrices deliberately
}


#endif RUBIKS_H
