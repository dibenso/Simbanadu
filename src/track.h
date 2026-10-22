#ifndef TRACK_H
#define TRACK_H

#include <stdlib.h>
#include "raylib.h"

typedef struct Track
{
    char *name;
    Vector3 *points;
    size_t numberOfPoints;
    size_t initialBallCount;
} Track;

#endif // TRACK_H