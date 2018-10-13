
#include <sphere.h>

Sphere *sphere_create(double x, double y, double z, double radius) {
  Sphere *sphere = malloc(sizeof(Sphere));
  sphere->x = x;
  sphere->y = y;
  sphere->z = z;

  sphere->r = radius;

  return sphere;
}//sphere_create

void sphere_rgba(Sphere *obj, float r, float g, float b, float a) {
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
void sphere__add_point_deg(Sphere *obj, int *index, double lat, double lon) {
  //Load an xyz point into the vertex array based off of lon/lat coordinates
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
void sphere_init_model(Sphere *obj, unsigned int lat_count, unsigned int lon_count) {
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

  //Store paramteres for future reference
  obj->lats = lat_count;
  obj->lons = lon_count;

  //Initialize loop variables
  float latDelta = 360.0 / (2*lat_count);
  float latStart = -90.0 + latDelta/2.0;
  float latEnd   =  90.0 - latDelta/2.0;
  float lat;   //Latitude circles from -90deg N to 90deg S (inclusive)
                                          //starts at -90->0->90

  float lonDelta = 360.0/(lon_count-1); //Same concept as for deltaLat
  float lon = 0;    //For each latitude, longitude goes from [0, 360]

  // Allocate memory: # of points around sphere (lon_count) for every row(lat_count);
    // as each vertex is xyz, the float array counts 3x the above.
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

void sphere__add_index(int *indices, int *index, int lons, int lat_i, int lon_i){
  //The absolute index (in obj->vertices) is the current index around the
    // horizontal (lon_i) + the # of points per row (lons) * the row index (lat_i)
  int vertex_i = lons*lat_i + lon_i;
  indices[(*index)++] = vertex_i;
}//sphere__add_index

int *sphere_ebo_indices(int *indicesCount, unsigned int lat_count, unsigned int lon_count){

  //Allocate memory for indices
  (*indicesCount) = 2 * lat_count * lon_count;
  int *indices = malloc( *indicesCount * sizeof(int));

  int index = 0;

  int lat_i, lon_i;
  for (lat_i=0; lat_i<lat_count; lat_i++){
    for (lon_i=0; lon_i<lon_count; lon_i++){
      sphere__add_index(indices, &index, lon_count, lat_i, lon_i);
      sphere__add_index(indices, &index, lon_count, (lat_i+1)%lat_count, lon_i);
    }
  }

  return indices;
}//sphere_ebo_indices

void sphere_attach_vao(Sphere *obj){

  //Create/bind sphere VAO
  glGenVertexArrays(1, &obj->VAO);
  glBindVertexArray(obj->VAO);

  //Create/bind and buffer VBO
  unsigned int VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, 3*obj->vertexCount*sizeof(float), obj->vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*) 0);
  glEnableVertexAttribArray(0);

  int *indices = sphere_ebo_indices(&obj->ebo_indices_c, obj->lats, obj->lons);

  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj->ebo_indices_c*sizeof(int), indices, GL_STATIC_DRAW);

  //Unbind sphere VAO
  glBindVertexArray(0);
}//sphere_attach_vao

void sphere_local_matrix(Sphere *obj, mat4x4 local){
  mat4x4_identity(local);
  float r = obj->r;
  mat4x4_translate_in_place(local, obj->x, obj->y, obj->z);
  mat4x4_scale_aniso(local, local, r, r, r);
}//sphere_get_local_matrix
