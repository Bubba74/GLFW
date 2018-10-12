
#ifndef SPHERE_H
#define SPHERE_H

#include <linmath.h>
#include <stdlib.h>

typedef struct {
  double x, y, z;

  double radius;
  vec4 rgba;

  unsigned int vertexCount;
  float *vertices;

} Sphere;


void sphere_create(double x, double y, double z, double radius);
void sphere_rgba(Sphere *obj, vec4 color);
void sphere_init_model(Sphere *obj, unsigned int lon_count, unsigned int lat_count);


#endif
