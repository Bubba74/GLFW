
#include <camera.h>
#include <stdio.h>

int cInstance = 0;
void cameraForward(struct Camera *cam, double speed){
  vec3 direction;
  vec3_scale(direction, cam->dir, speed);
  vec3_add(cam->pos, cam->pos, direction);

  cam->updateViewMatrix = 1;
}//cameraForward

void cameraStrafe(struct Camera *cam, double speed){
  vec3 up = {0, 1, 0};
  vec3 direction;
  vec3_mul_cross(direction, cam->dir, up);
  vec3_norm(direction, direction);
  vec3_scale(direction, direction, speed);
  vec3_add(cam->pos, cam->pos, direction);

  cam->updateViewMatrix = 1;
}//cameraStrafe

void cameraUp(struct Camera *cam, double speed){
  vec3 direction = {0, -1, 0};
  vec3_scale(direction, direction, -speed);
  vec3_add(cam->pos, cam->pos, direction);

  cam->updateViewMatrix = 1;
}//cameraUp

void cameraUpdateDirection(struct Camera *cam){
  cam->dir[0] = cosf(cam->yaw)*cosf(cam->pitch);
  cam->dir[1] = sinf(cam->pitch);
  cam->dir[2] = sinf(cam->yaw)*cosf(cam->pitch);

  cam->updateViewMatrix = 1;
}//cameraUpdateDirection

void cameraRotateByMouse(struct Camera *cam, double dmx, double dmy){
  if (dmx == 0 && dmy == 0)
    return;

  cam->yaw += CameraSensitivity*(dmx);
  cam->pitch -= CameraSensitivity*(dmy);

  float max_angle = 89/90.0 * M_PI/2;
  if (cam->pitch > max_angle)
    cam->pitch = max_angle;
  else if (cam->pitch < -max_angle)
    cam->pitch = -max_angle;

  cameraUpdateDirection(cam);
}//cameraRotateByMouse

void cameraRotate3d(struct Camera *cam, double pitch, double roll, double yaw){
  cam->pitch = pitch;
  cam->roll = roll;
  cam->yaw = yaw;

  cameraUpdateDirection(cam);
}//cameraRotate3d

void cameraGenerateViewMatrix(struct Camera *cam){
  if (!cam->updateViewMatrix) return;
  // mat4x4_identity(cam->viewMatrix);
  vec3 up			= {0, 1, 0};

  //Make sure cam->dir vector is fresh
  // cameraUpdateDirection();

  vec3 target;
  vec3_add(target, cam->pos, cam->dir);

  mat4x4_look_at(cam->viewMatrix, cam->pos, target, up);

  cam->updateViewMatrix = 0;
}//cameraGetViewMatrix

void cameraPosition3d(struct Camera *cam, double x, double y, double z){
  cam->pos[0] = x;
  cam->pos[1] = y;
  cam->pos[2] = z;

  cam->updateViewMatrix = 1;
}//cameraPosition3d

struct Camera *cameraGetNew(){
  //Allocate memory for new object
  struct Camera *newInstance = malloc(sizeof(struct Camera));

  return newInstance;
}//cameraGetNew
