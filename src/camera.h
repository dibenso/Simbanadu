#ifndef CAMERA_H
#define CAMERA_H

#include "raylib.h"

Vector3 GetCameraForward(Camera *);

// Returns the cameras up vector (normalized)
// Note: The up vector might not be perpendicular to the forward vector
Vector3 GetCameraUp(Camera *);

// Returns the cameras right vector (normalized)s
Vector3 GetCameraRight(Camera *);

// Moves the camera in its forward direction
void CameraMoveForward(Camera *, float);

// Moves the camera in its up direction
void CameraMoveUp(Camera *, float);
// Moves the camera target in its current right direction
void CameraMoveRight(Camera *, float);

#endif // CAMERA_H