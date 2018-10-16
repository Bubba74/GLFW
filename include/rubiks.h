
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
    0,1,2, 3,1,2,
    //Right
    1,5,3, 7,5,3,
    //Top
    4,5,0, 1,5,0,
    //Bot
    6,7,2, 3,7,2,
    //Left
    4,0,6, 2,0,6,
    //Back
    4,5,6, 7,5,6
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

  vec4 cubeColors[6] = {
		{1,0,0,1}, //Front
		{0,1,0,1}, //Right
		{1,1,1,1}, //Top
		{0,0,0,1}, //Bottom
		{1,1,0,1}, //Left
		{0,1,1,1}  //Back
	};

  vec4 black = {0,0,0,1};
  int face;
  for (face=0; face<6; face++){
    glUniform4fv(colorLoc, 1, cubeColors[face]);
    glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, (void*)(6*face*sizeof(int))  );
  }
  glBindVertexArray(0);

}//tile_render


typedef struct rubiks_struct {

  //Virtual cube, with rows starting at the top (0) to bottom(2)
  //Columns start at left (0) to right (2)

  vec3 pos, rot;
  mat4x4 transform;

  tile *tiles[27];
  vec4 faceColors[6];

} rubiks;

void rubiks__update_transform(rubiks *cube){
  mat4x4_translate(cube->transform, cube->pos[0], cube->pos[1], cube->pos[2]);
  mat4x4_rotate_X(cube->transform, cube->transform, cube->rot[0]); //pitch
  mat4x4_rotate_Y(cube->transform, cube->transform, cube->rot[1]); //yaw
  mat4x4_rotate_Z(cube->transform, cube->transform, cube->rot[2]); //roll
}//rubiks__update_transform

rubiks *rubiks_create(vec3 pos, vec4 colors[6]){
  rubiks *cube = malloc(sizeof(rubiks));
  vec3_dup(cube->pos, pos);
  cube->rot[0] = 0, cube->rot[1] = 0, cube->rot[2] = 0;

  int h;
  for (h=0; h<6; h++)
    vec4_dup(cube->faceColors[h], colors[h]);

  //Possible coordinates for tiles, notice 3*3*3 == 27 combos
  float xCoords[] = {-1.01, 0, 1.01};
  float yCoords[] = {-1.01, 0, 1.01};
  float zCoords[] = {-1.01, 0, 1.01};

  int i;
  for (i=0; i<27; i++){
    cube->tiles[i] = tile_create();

    vec3 pos = {xCoords[i%3], yCoords[(i%9)/3], zCoords[i/9]};
    tile_set_pos(cube->tiles[i], pos);

    int j;
    for (j=0; j<6; j++)
      tile_set_face_color(cube->tiles[i], j, cube->faceColors[j]);
  }
  rubiks__update_transform(cube);
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

void rubiks_rotate(rubiks *cube, vec3 cam_rot, double dx, double dy){
  //In the initial instance:
  // if (fabs(dy) > fabs(dx)){
    // cube->rot[0] -= dy/500.0; //pitch



  // } else {
    // double rel_angle = cube->rot[1] - cam_rot[1];
    cube->rot[1] -= dx/500.0; //yaw
    // printf("Rel_angle: %d\n", (int)(rel_angle/M_PI*90));
  // }
  rubiks__update_transform(cube);
}//rubiks_rotate

void getBounds(vec3 coords[4], float *minX, float *maxX, float *minY, float *maxY, float *minZ, float *maxZ){
  int v;
  for (v=0; v<4; v++){
    float *p = coords[v];
    if      (p[0] < *minX) *minX = p[0];
    else if (p[0] > *maxX) *maxX = p[0];
    if      (p[1] < *minY) *minY = p[1];
    else if (p[1] > *maxY) *maxY = p[1];
    if      (p[2] < *minZ) *minZ = p[2];
    else if (p[2] > *maxZ) *maxZ = p[2];
  }
}//getBounds

double rubiks_seek_face_tile(rubiks *cube, vec3 cam_pos, vec3 cam_dir, int *face_id, int *tile_id){
  float dx = 1.5, dy = 1.5, dz = 1.5;
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
    0,1,2, 3,1,2,
    //Right
    1,5,3, 7,5,3,
    //Top
    4,5,0, 1,5,0,
    //Bot
    6,7,2, 3,7,2,
    //Left
    4,0,6, 2,0,6,
    //Back
    4,5,6, 7,5,6
  };

  mat4x4 inverse; //of rubiks cube transformation
  // mat4x4_translate(inverse, -cube->pos[0], -cube->pos[1], -cube->pos[2]);
  // mat4x4_rotate_X(-cube->rot[0]);
  mat4x4_invert(inverse, cube->transform);

  int z;
  vec4 pos = {0,0,0,1}; for (z=0; z<3; z++) pos[z] = cam_pos[z];
  vec4 dir = {0,0,0,1}; for (z=0; z<3; z++) dir[z] = cam_dir[z];
  vec4 new_pos, new_dir;
  mat4x4_mul_vec4(new_pos, inverse, pos);
  mat4x4_mul_vec4(new_dir, inverse, dir);
  vec4_add(new_dir, new_dir, cube->pos);

  int print = 0;
  if (print) {
    printf("\n");
    printf("[ %.2f , %.2f , %.2f , %.2f] ==> ", pos[0], pos[1], pos[2], pos[3]);
    printf("[ %.2f , %.2f , %.2f , %.2f] ==> ", dir[0], dir[1], dir[2], dir[3]);
    printf("\n");
    printf("Pos: [ %.2f , %.2f , %.2f , %.2f]\n", new_pos[0], new_pos[1], new_pos[2], new_pos[3]);
    printf("Dir: [ %.2f , %.2f , %.2f , %.2f]\n", new_dir[0], new_dir[1], new_dir[2], new_dir[3]);
  }

  double dist = -1; //A really big number
  int faceTarget = -1;

  vec3 coords[4];
  // face, vertex, index
  int f, v, i;
  for (f=0; f<6; f++){

    //Load the 4 vertices of the cube's face
    for (v=0; v<4; v++){
      int i;
      for (i=0; i<3; i++){
        coords[v][i] = vertices[i+3*indices[6*f+v]];
      }//for each index of the vertex
    }//for each vertex

    //Get the min/max bounds for the square
    float MIN = -RAND_MAX, MAX = RAND_MAX;
    float minX=MAX, maxX=MIN, minY=MAX, maxY=MIN, minZ=MAX, maxZ=MIN;
    getBounds(coords, &minX, &maxX, &minY, &maxY, &minZ, &maxZ);

    //Seek distance to axis-aligned component of the tile
    double e = 0.01;
    double dist_to_tile;
    if (fabs(minX - maxX) < e)
      dist_to_tile = (minX-new_pos[0]) / new_dir[0];
    else if (fabs(minY - maxY) < e)
      dist_to_tile = (minY-new_pos[1]) / new_dir[1];
    else if (fabs(minZ - maxZ) < e)
      dist_to_tile = (minZ-new_pos[2]) / new_dir[2];
    else{
      printf("The rubik's tile is not orientated along a side\n");
      dist_to_tile = -1;
    }

    // printf("%d>X: [%.2f,%.2f]\tY: [%.2f,%.2f]\tZ: [%.2f,%.2f]", f,minX,maxX,minY,maxY,minZ,maxZ);


    //If there is no hope of reaching the target tile,
    //    or the tile is behind an already-contacted tile,
    //      continue to check the next tile

    // printf (" ==> %.2f\n", dist_to_tile);

    if (dist_to_tile < 0)
      continue;
    if (dist > 0 && dist_to_tile > dist)
      continue;

    //Check that the line contacts the square in that distance
    float xTarget = new_pos[0] + new_dir[0]*dist_to_tile;
    float yTarget = new_pos[1] + new_dir[1]*dist_to_tile;
    float zTarget = new_pos[2] + new_dir[2]*dist_to_tile;

    int xCheck = minX-e <= xTarget && xTarget <= maxX+e;
    int yCheck = minY-e <= yTarget && yTarget <= maxY+e;
    int zCheck = minZ-e <= zTarget && zTarget <= maxZ+e;
    if (xCheck && yCheck && zCheck){
      dist = dist_to_tile;
      faceTarget = f;
      // printf("Setting dist to %.2f and face to %d\n", dist, faceTarget);
    } else {
      int err = (  !xCheck?0: ( !yCheck?1:(!zCheck?2:3) )  );
      // printf ("Failed on %d\n", err);
    }

  }//for each face

  if (faceTarget > -1){
    *face_id = faceTarget;
  }

  return dist;
}//rubiks_seek_face
void rubiks_highlight(rubiks *cube, int tile_id, int face_id){

}//rubiks_highlight

#endif
