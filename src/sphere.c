
#include <sphere.h>

Sphere *sphere_create(double x, double y, double z, double radius)
{
  Sphere *sphere = malloc(sizeof(Sphere));
  sphere->x = x;
  sphere->y = y;
  sphere->z = z;

  sphere->radius = radius;

  return sphere;
}//sphere_create

void sphere_rgba(Sphere *obj, float r, float g, float b, float a)
{
  obj->rgba[0] = r;
  obj->rgba[1] = g;
  obj->rgba[2] = b;
  obj->rgba[3] = a;
}//sphere_rgba

float sphere__norm (float value){
  if (value < -1) return -1;
  if (value > 1) return 1;
  return value;
}//sphere__norm
void sphere__add_point_deg(Sphere *obj, int *index, double lat, double lon)
{   //Load an xyz point into the vertex array based off of lon/lat coordinates
  lat = M_PI * (lat / 180.0f);
  lon = M_PI * (lon / 180.0f);
  float x = sinf(lon)*cosf(lat);
  float y = sinf(lat);
  float z = cosf(lon)*cosf(lat);
  x = sphere__norm(x);
  y = sphere__norm(y);
  z = sphere__norm(z);
  obj->vertices[(*index)++] = x;
  obj->vertices[(*index)++] = y;
  obj->vertices[(*index)++] = z;
}//sphere__add_point

void sphere_init_model(Sphere *obj, unsigned int lat_count, unsigned int lon_count)
{
  //Check conditions
  if (obj == NULL){
    fprintf(stderr, "sphere_create_model: received NULL Sphere pointer\n");
    return;
  }
  if (lon_count < 3){
    fprintf(stderr, "sphere_create_model: long_count cannot be less than 3\n");
    return;
  }
  if (lat_count < 3){
    fprintf(stderr, "sphere_create_model: lat_count cannot be less than 3\n");
    return;
  }

  //Initialize loop variables
  float latDelta = 360.0 / (2*lat_count);
  float latStart = -90.0 + latDelta/2.0;
  float latEnd   =  90.0 - latDelta/2.0;
  float lat;   //Latitude circles from -90deg N to 90deg S (inclusive)
                                          //starts at -90->0->90

  float lonDelta = 360.0/(lon_count-1); //Same concept as for deltaLat
  float lon = 0;    //For each latitude, longitude goes from [0, 360]

  // OLD: Allocate memory for vertices: 1 for top, 1 for bottom, long_count for middle
  obj->vertexCount = lon_count * lat_count;
  obj->vertices = malloc(3*obj->vertexCount*sizeof(float));

  int ind = 0; //Vertex Index
  for (lat = latStart; lat<=latEnd; lat+=latDelta){
    for (lon=0; lon<=360.0; lon+=lonDelta)
    {
      sphere__add_point_deg(obj, &ind, lat, lon);
    }
  }

}//sphere_create_model
