#include "raylib.h"
#include "raymath.h"
#include "camera.h"

Vector3 GetCameraForward(Camera *camera)
{
    return Vector3Normalize(Vector3Subtract(camera->target, camera->position));
}

// Returns the cameras up vector (normalized)
// Note: The up vector might not be perpendicular to the forward vector
Vector3 GetCameraUp(Camera *camera)
{
    return Vector3Normalize(camera->up);
}

// Returns the cameras right vector (normalized)s
Vector3 GetCameraRight(Camera *camera)
{
    Vector3 forward = GetCameraForward(camera);
    Vector3 up = GetCameraUp(camera);

    return Vector3Normalize(Vector3CrossProduct(forward, up));
}

// Moves the camera in its forward direction
void CameraMoveForward(Camera *camera, float distance)
{
    Vector3 forward = GetCameraForward(camera);

    // Project vector onto world plane
    forward.y = 0;
    forward = Vector3Normalize(forward);

    // Scale by distance
    forward = Vector3Scale(forward, distance);

    // Move position and target
    camera->position = Vector3Add(camera->position, forward);
    camera->target = Vector3Add(camera->target, forward);
}

// Moves the camera in its up direction
void CameraMoveUp(Camera *camera, float distance)
{
    Vector3 up = GetCameraUp(camera);

    // Scale by distance
    up = Vector3Scale(up, distance);

    // Move position and target
    camera->position = Vector3Add(camera->position, up);
    camera->target = Vector3Add(camera->target, up);
}

// Moves the camera target in its current right direction
void CameraMoveRight(Camera *camera, float distance)
{
    Vector3 right = GetCameraRight(camera);

    // Project vector onto world plane
    right.y = 0;
    right = Vector3Normalize(right);

    // Scale by distance
    right = Vector3Scale(right, distance);

    // Move position and target
    camera->position = Vector3Add(camera->position, right);
    camera->target = Vector3Add(camera->target, right);
}