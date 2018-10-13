
#ifndef SPHERE_H
#define SPHERE_H

#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <linmath.h>

typedef struct {
  double x, y, z, r;

  vec4 rgba;

  unsigned int lats, lons;

  unsigned int vertexCount;
  float *vertices;

  unsigned int ebo_indices_c;
  unsigned int VAO;

} Sphere;


Sphere *sphere_create(double x, double y, double z, double radius);
void sphere_rgba(Sphere *obj, float r, float g, float b, float a);
void sphere_init_model(Sphere *obj, unsigned int lat_count, unsigned int lon_count);
int *sphere_ebo_indices(int *indicesCount, unsigned int lat_count, unsigned int lon_count);
void sphere_attach_vao(Sphere *obj);
void sphere_local_matrix(Sphere *obj, mat4x4 local);

#endif
