
#ifndef SPHERE_H
#define SPHERE_H

#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <linmath.h>

typedef struct {
  double x, y, z, r;
  vec3 pos;
  vec3 vel;
  vec3 rot;

  vec4 rgba;

  unsigned int lats, lons;

  unsigned int vertexCount;
  float *vertices;

  unsigned int ebo_indices_c;
  int *indices;
  unsigned int VAO;

  unsigned int textured, flipped;
  float *texVertices;

} Sphere;

//Instantiates the geometric framework of a sphere
Sphere *sphere_create(double x, double y, double z, double radius);
//Set rgba color of the sphere
void sphere_rgba(Sphere *obj, float r, float g, float b, float a);
//Initialize Sphere->vertices with the point cloud of the sphere, with given accuracy
void sphere_init_model(Sphere *obj, unsigned int lat_count, unsigned int lon_count);
//Used by sphere_attach_vao to create the integer indices of the coordinates to draw triangles
int *sphere_ebo_indices(int *indicesCount, unsigned int lat_count, unsigned int lon_count);
//Enable texturing of the sphere through an equirectangular texture.
void sphere_texture(Sphere *obj);
//Creates the spheres VAO (to be bound to render) with bound VBO and EBO
void sphere_attach_vao(Sphere *obj);
void sphere_local_matrix(Sphere *obj, mat4x4 local);

#endif
