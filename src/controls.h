#ifndef CONTROLS_H
#define CONTROLS_H

#include "raymath.h"

typedef struct Controls
{
    Vector3 rotation;
    Vector3 movement;
} Controls;

Controls processControls();

#endif // CONTROLS_H