

#ifndef CAMERA_H
#define CAMERA_H
#include "raylib.h"

// Camera rotation angles
extern float cameraPitch;
extern float cameraYaw;

void Init_Camera(Camera3D* camera);
void Update_Camera(Camera3D* camera);

#endif // CAMERA_H