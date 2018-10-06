
#ifndef CAMERA_H
#define CAMERA_H

#include <linmath.h>
#include <stdlib.h>

#define CameraSensitivity 0.005

struct Camera {
  int updateViewMatrix;

  vec3 pos;
  vec3 dir;

  double pitch, roll, yaw;

  mat4x4 viewMatrix;
};

void cameraForward(struct Camera *cam, double forwardMovement);
void cameraStrafe(struct Camera *cam, double horizontalMovement);
void cameraUp(struct Camera *cam, double verticalMovement);

void cameraRotateByMouse(struct Camera *cam, double deltaMouseX, double deltaMouseY);
void cameraRotate3d(struct Camera *cam, double pitch, double roll, double yaw);
void cameraPosition3d(struct Camera *cam, double newCameraX, double newCameraY, double newCameraZ);

void cameraGenerateViewMatrix(struct Camera *cam);
struct Camera *cameraGetNew();

#endif
