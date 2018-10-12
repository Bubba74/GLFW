
#ifndef SPHERE_H
#define SPHERE_H

#include <stdio.h>
#include <stdlib.h>
#include <linmath.h>

typedef struct {
  double x, y, z;

  double radius;
  vec4 rgba;

  unsigned int lats, lons;

  unsigned int vertexCount;
  float *vertices;

} Sphere;


Sphere *sphere_create(double x, double y, double z, double radius);
void sphere_rgba(Sphere *obj, float r, float g, float b, float a);
void sphere_init_model(Sphere *obj, unsigned int lat_count, unsigned int lon_count);
int *sphere_ebo_indices(int *indicesCount, unsigned int lat_count, unsigned int lon_count);

#endif
