
#include <camera.h>
#include <stdio.h>

int cInstance = 0;
void cameraForward(Camera *cam, double speed){
  //TODO Change this so the movement is perpendicular to the UP vector
  //(you can't simply walk into space)
  vec3 direction, right;
  vec3_mul_cross(right, cam->dir, cam->up);
  vec3_mul_cross(direction, cam->up, right);
  vec3_scale(direction, direction, speed);
  vec3_add(cam->pos, cam->pos, direction);

  cam->updateViewMatrix = 1;
}//cameraForward

void cameraStrafe(Camera *cam, double speed){
  vec3 direction;
  vec3_mul_cross(direction, cam->dir, cam->up);
  vec3_norm(direction, direction);
  vec3_scale(direction, direction, speed);
  vec3_add(cam->pos, cam->pos, direction);

  cam->updateViewMatrix = 1;
}//cameraStrafe

void cameraUp(Camera *cam, double speed){
  vec3 direction;
  vec3 up;
  vec3_scale(direction, cam->up, speed);
  vec3_add(cam->pos, cam->pos, direction);

  cam->updateViewMatrix = 1;
}//cameraUp

void cameraUpdateDirection(Camera *cam){
  cam->dir[0] = cosf(cam->yaw)*cosf(cam->pitch);
  cam->dir[1] = sinf(cam->pitch);
  cam->dir[2] = sinf(cam->yaw)*cosf(cam->pitch);

  cam->updateViewMatrix = 1;
}//cameraUpdateDirection

void cameraUpdateDirectionAroundTarget (Camera *cam, int dist_to_target){

  vec3 target_point;
  vec3 radius;
  vec3_scale(radius, cam->dir, dist_to_target);
  vec3_add(target_point, cam->pos, radius);

  cam->dir[0] = cosf(cam->yaw)*cosf(cam->pitch);
  cam->dir[1] = sinf(cam->pitch);
  cam->dir[2] = sinf(cam->yaw)*cosf(cam->pitch);

  //Update camera position, so position + dist_to_target = target_point
  vec3_scale(radius, cam->dir, dist_to_target);
  vec3_sub(cam->pos, target_point, radius);

  cam->updateViewMatrix = 1;
}//cameraUpdateDirectionAroundTarget


void camera__Rotate(Camera *cam, double dmx, double dmy){
  if (dmx == 0 && dmy == 0)
    return;

  cam->yaw += CameraSensitivity*(dmx);
  cam->pitch -= CameraSensitivity*(dmy);

  float max_angle = 89/90.0 * M_PI/2;
  if (cam->pitch > max_angle)
    cam->pitch = max_angle;
  else if (cam->pitch < -max_angle)
    cam->pitch = -max_angle;

}//camera__Rotate

void cameraRotateFromPos(Camera *cam, double dmx, double dmy){
  camera__Rotate(cam, dmx, dmy);
  cameraUpdateDirection(cam);
}//cameraRotateByMouse

void cameraRotateAroundTarget(Camera *cam, double dmx, double dmy, int dist_to_target){
  camera__Rotate(cam, dmx, dmy);
  cameraUpdateDirectionAroundTarget(cam, dist_to_target);
}//cameraRotateByMouse

void cameraSetUp(Camera *cam, vec3 newUp){
  cam->up[0] = newUp[0];
  cam->up[1] = newUp[1];
  cam->up[2] = newUp[2];
}//cameraSetUp



void cameraRotate3d(Camera *cam, double pitch, double roll, double yaw){
  cam->pitch = pitch;
  cam->roll = roll;
  cam->yaw = yaw;

  cameraUpdateDirection(cam);
}//cameraRotate3d

void cameraGenerateViewMatrix(Camera *cam){
  if (!cam->updateViewMatrix) return;
  // mat4x4_identity(cam->viewMatrix);

  //Make sure cam->dir vector is fresh
  // cameraUpdateDirection();

  vec3 target;
  vec3_add(target, cam->pos, cam->dir);

  mat4x4_look_at(cam->viewMatrix, cam->pos, target, cam->up);

  cam->updateViewMatrix = 0;
}//cameraGetViewMatrix

void cameraPosition3d(Camera *cam, double x, double y, double z){
  cam->pos[0] = x;
  cam->pos[1] = y;
  cam->pos[2] = z;

  cam->updateViewMatrix = 1;
}//cameraPosition3d

Camera *cameraGetNew(){
  //Allocate memory for new object
  Camera *newInstance = malloc(sizeof(Camera));
  cameraPosition3d(newInstance, 0, 0, 0);
  vec3 up = {0,1,0};
  cameraSetUp(newInstance, up);

  return newInstance;
}//cameraGetNew
