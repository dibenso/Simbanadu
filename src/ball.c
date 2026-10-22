#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include "ball.h"
#include "track.h"

static size_t ballTrackPointToTrackPointIndex(Ball ball)
{
    size_t trackPointIndex = (size_t)round(ball.trackPoint);

    return trackPointIndex;
}

Vector3 ballTrackPointToPoint(Track track, Ball ball)
{
    size_t trackPointIndex = ballTrackPointToTrackPointIndex(ball);
    Vector3 point = track.points[trackPointIndex > 0 ? trackPointIndex : 0];

    return point;
}

void roll(Ball *ball, Track track, float speedMultiplier)
{
    bool isAfterBeginning = ball->trackPoint >= 0.0f;
    ball->isBeforeEnd = ball->trackPoint <= (float)track.numberOfPoints - 1.0f - speedMultiplier;
    ball->shouldDrawBall = isAfterBeginning && ball->isBeforeEnd;

    if (ball->isBeforeEnd)
    {
        ball->trackPoint += speedMultiplier;
        size_t trackPointIndex = ballTrackPointToTrackPointIndex(*ball);
        Vector3 nextBallPoint = ballTrackPointToPoint(track, *ball);
        Vector3 previousBallPoint = trackPointIndex != 0 ? track.points[trackPointIndex - 1] : nextBallPoint;
        Vector3 ballRotationAxis = Vector3Zero();

        if (previousBallPoint.x > nextBallPoint.x)
        {
            ballRotationAxis.z = previousBallPoint.x - nextBallPoint.x;
        }

        if (previousBallPoint.x < nextBallPoint.x)
        {
            ballRotationAxis.z = previousBallPoint.x - nextBallPoint.x;
        }

        if (previousBallPoint.z > nextBallPoint.z)
        {
            ballRotationAxis.x = nextBallPoint.z - previousBallPoint.z;
        }

        if (previousBallPoint.z < nextBallPoint.z)
        {
            ballRotationAxis.x = nextBallPoint.z - previousBallPoint.z;
        }

        float angleRadians = DEG2RAD * (float)(trackPointIndex)*PI;
        Quaternion ballRotationQuaternion = QuaternionFromAxisAngle(ballRotationAxis, angleRadians);
        Matrix ballRotationMatrix = QuaternionToMatrix(ballRotationQuaternion);
        Matrix ballTranslationMatrix = MatrixTranslate(nextBallPoint.x, nextBallPoint.y, nextBallPoint.z);
        Matrix ballTransformationMatrix = MatrixMultiply(ballRotationMatrix, ballTranslationMatrix);
        ball->model.transform = ballTransformationMatrix;
    }
}