#ifndef GAME_H
#define GAME_H

#include <stdlib.h>
#include "track.h"
#include "ball.h"
#include "options.h"

#define BALL_BUFFER_SIZE 1024
#define SHOOTING_BALL_BUFFER_SIZE 5
#define BALL_MODEL_COUNT 4
#define BALL_SHOT_SOUND_SIZE 5
#define BALL_RADIUS 0.5f
#define DESTROYING_BALL_RANGE_BALL_BUFFER_SIZE 128
#define DESTROYING_BALL_RANGE_BUFFER_SIZE 256
#define DESTROYING_BALL_DEAD_ZONE 30.0f

typedef struct DestroyingBallRange
{
    size_t headBound;
    size_t tailBound;
    Ball balls[DESTROYING_BALL_RANGE_BALL_BUFFER_SIZE];
    size_t ballCount;
} DestroyingBallRange;

typedef struct Game
{
    Camera camera;
    Track track;
    Ball balls[BALL_BUFFER_SIZE];
    unsigned short int ballCount;
    Options options;
    Model trackModel;
    Model ballModels[BALL_MODEL_COUNT];
    Model cursorModel;
    Vector3 cursorPosition;
    Model planeModel;
    Model baseModel;
    Vector3 basePosition;
    Ball shootingBalls[SHOOTING_BALL_BUFFER_SIZE];
    Sound ballShotSounds[BALL_SHOT_SOUND_SIZE];
    bool isGameOver;
    DestroyingBallRange destroyingBallRanges[DESTROYING_BALL_RANGE_BUFFER_SIZE];
    unsigned short int destroyingBallRangeCount;
    /**
     * Determines the maximum amount of space between each track ball
     * so they are barely connected (colliding).
     *
     * See initializeBallSizeToTrackPoint()
     */
    float ballSizeToTrackPoint;
} Game;

static void initializeBallSizeToTrackPoint(Game *);

static void antiAliasModelTextures(Game *);

static int findHeadIndexOfLastGroupOfDisconnectedTrackBalls(Game *);

static void removeConsecutiveTrackBallsAtPosition(Game *, size_t);

static bool isBallTypeOnTrack(Game *, unsigned short int);

static unsigned short int randomBallTypeFromBallOnTrackSet(const Game *);

static void initializeTrackBall(Game *, size_t);

static void initializeShootingBall(Game *, size_t);

static void initializeBalls(Game *);

static void initializeGameState(Game *);

static int insertBallOnTrackAtPositionWithTrackPoint(Game *, Ball, size_t, size_t);

static void initializeWindow();

static Camera initializeCamera();

Game initializeGame();

void deinitializeGame(const Game *);

void processFrame(Game *);

#endif // GAME_H