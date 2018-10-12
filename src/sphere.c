
#include <sphere.h>

void sphere_create(double x, double y, double z, double radius)
{
  Sphere *sphere = malloc(sizeof(Sphere));
  sphere->x = x;
  sphere->y = y;
  sphere->z = z;

  sphere->radius = radius;

  return sphere;
}//sphere_create

void sphere_rgba(Sphere *obj, float r, float g, float b)
{
  obj->rgba = vec4(r, g, b, a);
}//sphere_rgba

void sphere__add_point(Sphere *obj, int *index, double lon, double lat)
{   //Load an xyz point into the vertex array based off of lon/lat coordinates
  obj->vertices[(*index)++] = sinf(lon)*cosf(lat);
  obj->vertices[(*index)++] = sinf(lat);
  obj->vertices[(*index)++] = cosf(lon)*cosf(lat);
}//sphere__add_point

void sphere_create_model(Sphere *obj, unsigned int lon_count, unsigned int lat_count)
{
  //Check conditions
  if (obj == NULL){
    fprintf(stderr, "sphere_create_model: received NULL Sphere pointer\n");
    return;
  }
  if (long_count < 3){
    fprintf(stderr, "sphere_create_model: long_count cannot be less than 3\n");
    return;
  }
  if (lat_count < 3){
    fprintf(stderr, "sphere_create_model: lat_count cannot be less than 3\n");
    return;
  }

  //Initialize loop variables
  float latDelta = 360.0 / (2*lat_count);
  float latStart = -90.0 + latDelta/2;
  float latEnd   =  90.0 - latDelta/2;
  float lat;   //Latitude circles from -90deg N to 90deg S (inclusive)
                                          //starts at -90->0->90

  float lonDelta = 360.0/(lon_count-1); //Same concept as for deltaLat
  float lon = 0;    //For each latitude, longitude goes from [0, 360]

  //Allocate memory for vertices: 1 for top, 1 for bottom, long_count for middle
  int vertexCount = lon_count * lat_count;
  obj->vertexCount = vertexCount;
  obj->vertices = malloc(3*vertexCount*sizeof(float));

  int ind = 0; //Vertex Index
  for (lat = latStart; lat<=latEnd; lat+=latDelta){
    for (lon=0; lon<=360.0; lon+=lonDelta)
    {
      sphere__add_point(obj, &ind, lon, lat);
      //For the top and bottom, only append the one point
      if (lat == latStart || lat == latEnd)
        break;
    }
  }

}//sphere_create_model
