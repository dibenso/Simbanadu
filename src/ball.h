#ifndef BALL_H
#define BALL_H

#include <stdlib.h>
#include "raylib.h"
#include "track.h"
#include "options.h"

typedef struct Ball
{
    /**
     * model:
     * The model used by both track and shooting balls
     */
    Model model;
    /**
     * trackPoint:
     * If this is a track ball, where on the track is it?
     */
    float trackPoint;
    /**
     * ballType:
     * Used by both track and shooting balls to identify
     * which ball type (color / texture) it is
     */
    unsigned short int ballType;
    /**
     * position:
     * Where the shooting ball is currently located
     */
    Vector3 position;
    /**
     * target:
     * Where the shooting ball is heading
     */
    Vector3 target;
    /**
     * isShooting:
     * If this is a shooting ball, is it currently shooting
     */
    bool isShooting;
    /**
     * shouldDrawBall:
     * Ball should only be drawn when it's after the beginning
     * and before the end of the track
     */
    bool shouldDrawBall;
    /**
     * isBeforeEnd:
     * For checking if ball has reached the end of the track.
     *
     * Used for signaling when it's game over
     */
    bool isBeforeEnd;
} Ball;

static size_t ballTrackPointToTrackPointIndex(Ball);

Vector3 ballTrackPointToPoint(Track, Ball);

void roll(Ball *, Track, float);

#endif // BALL_H