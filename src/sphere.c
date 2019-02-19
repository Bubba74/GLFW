
#include <assert.h>
#include <sphere.h>
#include <linmath.h>

Sphere *sphere_create(double x, double y, double z, double radius) {
  Sphere *sphere = malloc(sizeof(Sphere));
  sphere->x = x;
  sphere->y = y;
  sphere->z = z;

  sphere->pos[0] = x; sphere->pos[1] = y; sphere->pos[2] = z;


  sphere->r = radius;

  sphere->vel[0] = 0;  sphere->vel[1] = 0;  sphere->vel[2] = 0;
  sphere->rot[0] = 0;  sphere->rot[1] = 0;  sphere->rot[2] = 0;

  sphere->textured = 0; //Default: not textured
  sphere->flipped = 0;

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

void sphere__add_point_deg(Sphere *obj, int *index, double lat, double lon, double radius) {
  //Load an xyz point into the vertex array based off of lon/lat coordinates
  lat = M_PI * (lat / 180.0f);
  lon = M_PI * (lon / 180.0f);
  //Adjust for radius != 1
  lat = lat/radius;
  lon = lon/radius;
  //Find center of sphere
  vec3 center = {0,0,-radius+1};
  float x = radius*sinf(lon)*cosf(lat);
  float y = radius*sinf(lat);
  float z = radius*cosf(lon)*cosf(lat);
  // x = sphere__norm(x);
  // y = sphere__norm(y);
  // z = sphere__norm(z);
  obj->vertices[(*index)++] = x+center[0];
  obj->vertices[(*index)++] = y+center[1];
  obj->vertices[(*index)++] = z+center[2];
}//sphere__add_point

//Add TexCoord based off of (DEGREES) lat and lon angles
void sphere__add_tex_deg(Sphere *obj, int *index, double lat, double lon) {
  //UV Coordinates go from Bottom-Left at (0,0) to Top-Right at (1,1)
  /*
    (0,1)--(1,1)
      |      |
      |      |
    (0,0)--(1,0)
  */

  //Convert (lat, lon) to (x,y) coordinates of a GLFW texture
  float x = lon / 360.0f;
  float y = (-lat+90.0f) / 180.0f;

  obj->texVertices[(*index)++] = x;
  obj->texVertices[(*index)++] = y;

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
  float latDelta = 180.0 / obj->lats; //starts at -90->0->90
  float latStart = -90.0 + latDelta/2.0;

  float lonDelta = 360.0/(obj->lons-1); //Same concept as for deltaLat
  float lonStart =   0.0 + lonDelta/2.0;

  // Allocate memory: # of points around sphere (lon_count) for every row(lat_count);
    // as each vertex is xyz, the float array counts 3x the above.
  obj->vertexCount = (obj->lons+1) * obj->lats;
  obj->vertices = malloc(3*obj->vertexCount*sizeof(float));

  if (obj->textured)
    obj->texVertices = malloc (2*obj->vertexCount*sizeof(float));

  int vertIndex = 0; //Vertex Index
  int texIndex = 0; //TexCoord index

  double latDeg, lonDeg;
  int lat_i, lon_i;
  for (lat_i=0; lat_i < obj->lats; lat_i++){
    // The extra lon_i accounts for wrapping textures properly around the 360deg boundary
      // This extra point has the same coordinate as the first, but a different TexCoord
    for (lon_i=0; lon_i <= obj->lons; lon_i++){
      latDeg = latStart + lat_i*latDelta;
      lonDeg = lonStart + lon_i*lonDelta;
      sphere__add_point_deg(obj, &vertIndex, latDeg, lonDeg, 1.0);
      if (obj->textured)
        sphere__add_tex_deg(obj, &texIndex, latDeg, obj->flipped?-lonDeg:lonDeg);
    }
  }
}//sphere_create_model

void sphere_update_model(Sphere *obj, float progressPercent) {

  //Check conditions
  if (obj == NULL){
    fprintf(stderr, "sphere_create_model: received NULL Sphere pointer\n");
    return;
  }
  if (progressPercent < 0){
    fprintf(stderr, "sphere_update_model: progressPercent cannot be less than 0\n");
    progressPercent = 0;

    // return;
  }
  if (progressPercent > 1)
    progressPercent = 1;

  //Transition smoothly between an equirectangular plane and a sphere
  /*

  t == 0: r = infinity -> sphere of radius infinity with edge at (0,0,1)
  t == 1: r = 1 -> sphere of radius 1 with edge at (0,0,1)
  use same formula in sphere_init_model except with a variable radius and center
  the center should be normal to the equirectangular plane and pass through the
  (latitude, longitude) point (0,0) which has the (x,y,z) coordinate (0,0,1)

  */
  if (progressPercent == 0)
    progressPercent = 0.0000001;
  float radius = 1/progressPercent;
  //Initialize loop variables
  float latDelta = 180.0 / obj->lats; //starts at -90->0->90
  float latStart = -90.0 + latDelta/2.0;

  float lonDelta = 360.0/(obj->lons-1); //Same concept as for deltaLat
  float lonStart =   -180.0 + lonDelta/2.0;

  int vertIndex = 0; //Vertex Index
  double latDeg, lonDeg;
  int lat_i, lon_i;
  for (lat_i=0; lat_i < obj->lats; lat_i++){
    // The extra lon_i accounts for wrapping textures properly around the 360deg boundary
      // This extra point has the same coordinate as the first, but a different TexCoord
    for (lon_i=0; lon_i <= obj->lons; lon_i++){
      latDeg = latStart + lat_i*latDelta;
      lonDeg = lonStart + lon_i*lonDelta;
      sphere__add_point_deg(obj, &vertIndex, latDeg, lonDeg, radius);
    }
  }

  glBindVertexArray(obj->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, obj->VBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, 3*obj->vertexCount*sizeof(float), obj->vertices);

}//sphere_create_model

void sphere__add_index(int *indices, int *index, int lons, int lat_i, int lon_i){
  //The absolute index (in obj->vertices) is the current index around the
    // horizontal (lon_i) + the # of points per row (lons) * the row index (lat_i)
  int vertex_i = (lons+1)*lat_i + lon_i;
  indices[(*index)++] = vertex_i;
}//sphere__add_index

int *sphere_ebo_indices(int *indicesCount, unsigned int lat_count, unsigned int lon_count){

  //Allocate memory for indices
  *indicesCount = (lat_count-1) * 2*lon_count;
  int *indices = malloc( *indicesCount * sizeof(int));

  int index = 0;

  int goRight = 1;
  int lat_i, lon_i;
  for (lat_i=0; lat_i<lat_count-1; lat_i++){
    for (lon_i=0; lon_i<lon_count; lon_i++){
      if (goRight){
        sphere__add_index(indices, &index, lon_count, lat_i,   lon_i);
        sphere__add_index(indices, &index, lon_count, lat_i+1, lon_i);
      } else {
        sphere__add_index(indices, &index, lon_count, lat_i,   lon_count-lon_i-1);
        sphere__add_index(indices, &index, lon_count, lat_i+1, lon_count-lon_i-1);
      }
    }
    //Alternate between going right and going right between layers
    goRight = 1-goRight;
  }

  return indices;
}//sphere_ebo_indices

//Enable texturing of the sphere through an equirectangular texture.
void sphere_texture(Sphere *obj) {
  obj->textured = 1;
  return;
} // sphere_texture

void sphere_attach_vao(Sphere *obj){

  //Create/bind sphere VAO
  glGenVertexArrays(1, &obj->VAO);
  glBindVertexArray(obj->VAO);

  //Create/bind and buffer VBO
  unsigned int VBO;
  glGenBuffers(1, &VBO);
  obj->VBO = VBO;
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  long vertFloats = obj->vertexCount*sizeof(float);
  if (obj->textured){
    glBufferData(GL_ARRAY_BUFFER, 5*vertFloats, NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, 3*vertFloats, obj->vertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    glBufferSubData(GL_ARRAY_BUFFER, 3*vertFloats, 2*vertFloats, obj->texVertices);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*) (3*vertFloats));
    glEnableVertexAttribArray(1);
  } else {
    glBufferData(GL_ARRAY_BUFFER, 3*vertFloats, obj->vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
  }

  obj->indices = sphere_ebo_indices(&obj->ebo_indices_c, obj->lats, obj->lons);

  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj->ebo_indices_c*sizeof(int), obj->indices, GL_STATIC_DRAW);

  //Unbind sphere VAO
  glBindVertexArray(0);
}//sphere_attach_vao

void sphere_local_matrix(Sphere *obj, mat4x4 local){
  mat4x4_identity(local);
  float r = obj->r;

  mat4x4_rotate_X(local, local, obj->rot[0]);
  mat4x4_rotate_Y(local, local, obj->rot[1]);
  mat4x4_rotate_Z(local, local, obj->rot[2]);

  mat4x4_translate_in_place(local, obj->x, obj->y, obj->z);
  mat4x4_scale_aniso(local, local, r, r, r);
}//sphere_get_local_matrix
