#include "raylib.h"
#include "controls.h"

Controls processControls()
{
    Vector2 mouseDelta = GetMouseDelta();
    // sensitivity factor: 0.01 lowest, 0.1 highest, 0.05 middle
    Vector3 rotation = {mouseDelta.x * 0.05f, mouseDelta.y * 0.05f, 0.0f};
    Vector3 movement = Vector3Zero();

    if (IsKeyDown(KEY_W))
    {
        movement.x += 0.1f;
    }
    if (IsKeyDown(KEY_A))
    {
        movement.y -= 0.1f;
    }
    if (IsKeyDown(KEY_S))
    {
        movement.x -= 0.1f;
    }
    if (IsKeyDown(KEY_D))
    {
        movement.y += 0.1f;
    }

    Controls controls = {rotation, movement};

    return controls;
}