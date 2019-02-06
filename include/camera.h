
#ifndef CAMERA_H
#define CAMERA_H

#include <linmath.h>
#include <stdlib.h>

#define CameraSensitivity 0.005

typedef struct camera_struct {
  int updateViewMatrix;

  vec3 pos;
  vec3 dir;
  vec3 up;

  double pitch, roll, yaw;

  mat4x4 viewMatrix;
} Camera;

void cameraForward(Camera *cam, double forwardMovement);
void cameraStrafe(Camera *cam, double horizontalMovement);
void cameraUp(Camera *cam, double verticalMovement);

void cameraRotateAroundTarget(Camera *cam, double deltaMouseX, double deltaMouseY, int dist_to_target);
void cameraRotateFromPos(Camera *cam, double deltaMouseX, double deltaMouseY);
void cameraSetUp(Camera *cam, vec3 newUp);

void cameraRotate3d(Camera *cam, double pitch, double roll, double yaw);
void cameraPosition3d(Camera *cam, double newCameraX, double newCameraY, double newCameraZ);

void cameraGenerateViewMatrix(Camera *cam);
Camera *cameraGetNew();

#endif
