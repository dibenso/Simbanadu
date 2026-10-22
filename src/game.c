#include <stdlib.h>
#include <math.h>
#include "raylib.h"
#include "raymath.h"
#include "track.h"
#include "ball.h"
#include "controls.h"
#include "options.h"
#include "game.h"
#include "tracks/track1.h"
#include "debug.h"

static int findHeadIndexOfLastGroupOfDisconnectedTrackBalls(Game *game)
{
    for (size_t i = game->ballCount - 1; i > 0; i--)
    {
        float trackPointDelta = game->balls[i - 1].trackPoint - game->balls[i].trackPoint;

        if (trackPointDelta > game->ballSizeToTrackPoint)
        {
            return i;
        }
    }

    return -1;
}

/**
 * NOTE: Same function as above but using collision detection instead. Is this more flexible / reliable?
static int findHeadIndexOfLastGroupOfDisconnectedTrackBalls(Game *game)
{
    for (size_t i = game->ballCount - 1; i > 0; i--)
    {
        Vector3 backTrackBallPosition = ballTrackPointToPoint(game->track, game->balls[i]);
        Vector3 frontTrackBallPosition = ballTrackPointToPoint(game->track, game->balls[i - 1]);
        bool areTrackBallsConnected = CheckCollisionSpheres(backTrackBallPosition, BALL_RADIUS, frontTrackBallPosition, BALL_RADIUS);

        if (!areTrackBallsConnected)
        {
            return i;
        }
    }

    return -1;
}
 */

static void removeConsecutiveTrackBallsAtPosition(Game *game, size_t index)
{
    if (index < 0 || index >= game->ballCount)
    {
        return;
    }

    unsigned short int ballType = game->balls[index].ballType;
    unsigned short int towardsHead = index;
    unsigned short int towardsTail = index;

    /**
     * Consecutive (and connecting) going towards the head ball on track
     */
    while (towardsHead > 0 && game->balls[towardsHead - 1].ballType == ballType && (game->balls[towardsHead - 1].trackPoint - game->balls[towardsHead].trackPoint) == game->ballSizeToTrackPoint)
    {
        towardsHead--;
    }

    /**
     * Consecutive (and connecting) going towards the tail ball on track
     */
    while (towardsTail + 1 < game->ballCount && game->balls[towardsTail + 1].ballType == ballType && (game->balls[towardsTail].trackPoint - game->balls[towardsTail + 1].trackPoint) == game->ballSizeToTrackPoint)
    {
        towardsTail++;
    }

    unsigned short int numberOfConsecutiveTrackBalls = towardsTail - towardsHead + 1;

    if (numberOfConsecutiveTrackBalls < 3)
    {
        return;
    }

    game->destroyingBallRangeCount++;
    size_t destroyingBallRangeIndex = game->destroyingBallRangeCount - 1;
    game->destroyingBallRanges[destroyingBallRangeIndex].headBound = towardsHead;
    game->destroyingBallRanges[destroyingBallRangeIndex].tailBound = towardsTail;
    game->destroyingBallRanges[destroyingBallRangeIndex].ballCount = numberOfConsecutiveTrackBalls;
    size_t currentDestroyingBallRangeBallIndex = 0;

    for (size_t i = towardsHead; i <= towardsTail; i++)
    {
        game->destroyingBallRanges[destroyingBallRangeIndex].balls[currentDestroyingBallRangeBallIndex] = game->balls[i];
        game->destroyingBallRanges[destroyingBallRangeIndex].balls[currentDestroyingBallRangeBallIndex].model.transform = MatrixIdentity();
        Vector3 destroyedBallPosition = ballTrackPointToPoint(game->track, game->balls[i]);
        game->destroyingBallRanges[destroyingBallRangeIndex].balls[currentDestroyingBallRangeBallIndex].position = destroyedBallPosition;
        Vector3 destroyingBallTarget = Vector3Subtract(destroyedBallPosition, game->basePosition);
        Vector3 scaledDestroyingBallTarget = Vector3Scale(destroyingBallTarget, 120.0f / Vector3Length(destroyingBallTarget));
        game->destroyingBallRanges[destroyingBallRangeIndex].balls[currentDestroyingBallRangeBallIndex].target = scaledDestroyingBallTarget;
        currentDestroyingBallRangeBallIndex++;
    }

    for (size_t i = towardsTail + 1; i < game->ballCount; i++)
    {
        game->balls[i - numberOfConsecutiveTrackBalls] = game->balls[i];
    }

    game->ballCount -= numberOfConsecutiveTrackBalls;
}

static bool isBallTypeOnTrack(Game *game, unsigned short int ballType)
{
    for (size_t i = 0; i < game->ballCount; i++)
    {
        if (game->balls[i].ballType == ballType)
        {
            return true;
        }
    }

    return false;
}

static unsigned short int randomBallTypeFromBallOnTrackSet(const Game *game)
{
    int modelIndex;
    bool isValidShootingBall = false;

    do
    {
        modelIndex = GetRandomValue(0, BALL_MODEL_COUNT - 1);

        for (size_t j = 0; j < game->track.initialBallCount; j++)
        {
            isValidShootingBall = game->balls[j].ballType == modelIndex;

            if (isValidShootingBall)
            {
                break;
            }
        }
    } while (!isValidShootingBall);

    return modelIndex;
}

static void initializeTrackBall(Game *game, size_t trackBallIndex)
{
    /**
     * Do not allow more than 2 of the same balls to be initialized next to each other
     * int ballType;
     * do
     * {
     *  ballType = GetRandomValue(0, BALL_MODEL_COUNT - 1);
     * } while (trackBallIndex > 1 && ballType == game->balls[trackBallIndex - 1].ballType && ballType == game->balls[trackBallIndex - 2].ballType);
     */

    int ballType = GetRandomValue(0, BALL_MODEL_COUNT - 1);

    game->balls[trackBallIndex].ballType = ballType;
    game->balls[trackBallIndex].shouldDrawBall = false;
    game->balls[trackBallIndex].isBeforeEnd = false;
    game->balls[trackBallIndex].model = game->ballModels[ballType];
    game->balls[trackBallIndex].trackPoint = 0.0f - ((float)trackBallIndex * game->ballSizeToTrackPoint);
}

static void initializeShootingBall(Game *game, size_t shootingBallsIndex)
{
    unsigned short int ballType = randomBallTypeFromBallOnTrackSet(game);
    game->shootingBalls[shootingBallsIndex].isShooting = false;
    game->shootingBalls[shootingBallsIndex].ballType = ballType;
    game->shootingBalls[shootingBallsIndex].model = game->ballModels[ballType];
    game->shootingBalls[shootingBallsIndex].position = (Vector3){game->basePosition.x, game->basePosition.y + BALL_RADIUS - (BALL_RADIUS * (float)shootingBallsIndex), game->basePosition.z - 1.0f + (1.0f * (float)shootingBallsIndex)};
}

/**
 * Determines the maximum amount of space between each track ball
 * so they are barely connected (colliding).
 */
static void initializeBallSizeToTrackPoint(Game *game)
{
    Ball collisionBall1 = {0};
    Ball collisionBall2 = {0};
    collisionBall1.trackPoint = 0.0f;
    collisionBall2.trackPoint = 0.0f;
    bool isCollision = true;

    while (isCollision)
    {
        /**
         * As long as the balls are still colliding, update game->ballSizeToTrackPoint.
         *
         * NOTE: - (BALL_RADIUS * 2.0f) is used to make each track ball appear a bit more
         * close to each other.
         */
        game->ballSizeToTrackPoint = (float)round((float)collisionBall2.trackPoint) - (BALL_RADIUS * 2.0f);
        /**
         * Get vectors for the positions of both balls and check if they are colliding.
         * If they are not colliding, the loop will continue.
         * Otherwise, game->ballSizeToTrackPoint will have the previous collisionBall2.trackPoint
         * before it was incremented.
         */
        Vector3 ball1PointOnTrack = ballTrackPointToPoint(game->track, collisionBall1);
        Vector3 ball2PointOnTrack = ballTrackPointToPoint(game->track, collisionBall2);
        isCollision = CheckCollisionSpheres(ball1PointOnTrack, BALL_RADIUS, ball2PointOnTrack, BALL_RADIUS);
        /**
         * Move ball 2 further down the track
         */
        collisionBall2.trackPoint += 0.01f;
    }
}

static void initializeBalls(Game *game)
{
    for (size_t i = 0; i < game->track.initialBallCount; i++)
    {
        initializeTrackBall(game, i);
    }

    for (size_t i = 0; i < SHOOTING_BALL_BUFFER_SIZE; i++)
    {
        initializeShootingBall(game, i);
    }
}

static void initializeGameState(Game *game)
{
    game->options.speedMultiplier = 0.5f;
    game->isGameOver = false;
    game->track.name = "track1";
    game->track.points = track1;
    game->track.numberOfPoints = sizeof(track1) / sizeof(track1[0]);
    game->track.initialBallCount = 56;
    game->ballCount = game->track.initialBallCount;
    game->basePosition = (Vector3){-4.0f, -0.6f, 5.0f};
    game->cursorPosition = game->basePosition;
    game->destroyingBallRangeCount = 0;

    initializeBallSizeToTrackPoint(game);
    initializeBalls(game);
}

static int insertBallOnTrackAtPosition(Game *game, Ball ball, size_t shootingBallsIndex, size_t collisionIndex)
{
    /**
     * Collision occurred with ball on track.
     * Was it at the front or back radius of the ball?
     *
     * In front -> position to insert is collisionIndex
     * In back -> position to insert is collisionIndex + 1
     */
    Ball frontBallForComparison = game->balls[collisionIndex];
    Ball backBallForComparison = game->balls[collisionIndex];
    frontBallForComparison.trackPoint += game->ballSizeToTrackPoint;
    backBallForComparison.trackPoint -= game->ballSizeToTrackPoint;
    Vector3 frontBallForComparisonPointOnTrack = ballTrackPointToPoint(game->track, frontBallForComparison);
    Vector3 backBallForComparisonPointOnTrack = ballTrackPointToPoint(game->track, backBallForComparison);
    float distanceBetweenShootingBallAndFrontBallForComparison = Vector3Distance(frontBallForComparisonPointOnTrack, game->shootingBalls[shootingBallsIndex].position);
    float distanceBetweenShootingBallAndBackBallForComparison = Vector3Distance(backBallForComparisonPointOnTrack, game->shootingBalls[shootingBallsIndex].position);
    size_t indexToInsertBallOnTrack = distanceBetweenShootingBallAndFrontBallForComparison < distanceBetweenShootingBallAndBackBallForComparison ? collisionIndex : collisionIndex + 1;

    if (indexToInsertBallOnTrack < 0 || indexToInsertBallOnTrack > game->ballCount)
    {
        fprintf(stderr, "Invalid insertion of track ball at position: %zu\n", indexToInsertBallOnTrack);

        return -1;
    }

    if (game->ballCount == BALL_BUFFER_SIZE)
    {
        fprintf(stderr, "Max track balls reached\n");

        return -1;
    }

    /**
     * Shift balls ahead of indexToInsertBallOnTrack to make room for this
     * new one to insert
     */
    for (size_t i = game->ballCount; i > indexToInsertBallOnTrack; i--)
    {
        game->balls[i] = game->balls[i - 1];
    }

    /**
     * Assign new ball and increment ball count
     */
    game->balls[indexToInsertBallOnTrack] = ball;
    game->balls[indexToInsertBallOnTrack].shouldDrawBall = true;
    game->ballCount++;

    /**
     * Initialize new shooting ball to replace the one that was just added to the track
     */
    initializeShootingBall(game, shootingBallsIndex);

    /**
     * In this case, the ball is being inserted at the terminal head or tail.
     *
     * There is no need to shift track points of balls on the track to make
     * room for this new one, so we can return early after setting the track
     * point of this new ball inserted.
     */
    if (indexToInsertBallOnTrack == 0 || indexToInsertBallOnTrack == game->ballCount - 1)
    {
        game->balls[indexToInsertBallOnTrack].trackPoint = indexToInsertBallOnTrack == 0 ? game->balls[1].trackPoint + game->ballSizeToTrackPoint : game->balls[game->ballCount - 2].trackPoint - game->ballSizeToTrackPoint;

        return indexToInsertBallOnTrack;
    }

    /**
     * If shooting ball is being inserted in front of the ball on the track it collided with:
     * Set its track point to the ball just behind it + game->ballSizeToTrackPoint
     */
    if (indexToInsertBallOnTrack == collisionIndex)
    {
        game->balls[indexToInsertBallOnTrack].trackPoint = game->balls[indexToInsertBallOnTrack + 1].trackPoint + game->ballSizeToTrackPoint;
    }
    /**
     * Otherwise, the shooting ball is being inserted behind the ball on the track it collided with:
     * In this case, the ball being inserted falls into 2 categories:
     */
    else
    {
        /**
         * Category 1 - The track ball behind the shooting ball being inserted has a gap > game->ballSizeToTrackPoint * 2.0f:
         * The track point of the ball will be set to the track ball in front of it - game->ballSizeToTrackPoint
         */
        if (game->balls[indexToInsertBallOnTrack - 1].trackPoint - game->balls[indexToInsertBallOnTrack + 1].trackPoint > game->ballSizeToTrackPoint * 2.0f)
        {
            game->balls[indexToInsertBallOnTrack].trackPoint = game->balls[indexToInsertBallOnTrack - 1].trackPoint - game->ballSizeToTrackPoint;
        }
        /**
         * Category 2 - The track ball behind the shooting ball being inserted has a gap <= game->ballSizeToTrackPoint * 2.0f
         * The track point of the ball will be set to the track ball in front of it.
         *
         * NOTE: No subtraction.
         */
        else
        {
            game->balls[indexToInsertBallOnTrack].trackPoint = game->balls[indexToInsertBallOnTrack - 1].trackPoint;
        }
    }

    /**
     * For all balls ahead of the new one inserted:
     */
    for (size_t i = indexToInsertBallOnTrack; i > 0; i--)
    {
        /**
         * Shift balls starting from the position of the ball that was just inserted -1 and moving
         * towards the head. For each iteration, a check is made to check if the current ball and
         * the next one are overlapping. If they are overlapping, the current ball will be shifted
         * by the amount they are overlapping.
         */
        if (game->balls[i - 1].trackPoint - game->balls[i].trackPoint <= game->ballSizeToTrackPoint)
        {
            game->balls[i - 1].trackPoint += game->ballSizeToTrackPoint - (game->balls[i - 1].trackPoint - game->balls[i].trackPoint);
        }
    }

    return indexToInsertBallOnTrack;
}

static void initializeWindow()
{
    InitWindow(1280, 720, "Simbanadu");
    // ToggleFullscreen();
}

static Camera initializeCamera()
{
    Camera camera = {0};
    /*
    camera.position = (Vector3){0.0f, 40.0f, 0.0f}; // Camera position (10 units up)
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};    // Camera looking at point (the origin)
    camera.up = (Vector3){0.0f, 0.0f, -1.0f};       // Camera up vector (forward is now down)
    camera.fovy = 45.0f;                            // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;         // Camera projection
    */
    camera.position = (Vector3){0.0f, 40.0f, 40.0f};
    camera.target = Vector3Zero();
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 22.5f;
    camera.projection = CAMERA_PERSPECTIVE;

    return camera;
}

Game initializeGame()
{
    initializeWindow();
    SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
    DisableCursor();
    SetTargetFPS(60);
    InitAudioDevice();

    Game game = {0};
    game.camera = initializeCamera();
    game.trackModel = LoadModel("resources/models/tracks/track1/track1.obj");
    GenTextureMipmaps(&game.trackModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture);
    game.baseModel = LoadModel("resources/models/bases/base1/base1.obj");
    GenTextureMipmaps(&game.baseModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture);
    game.cursorModel = LoadModel("resources/models/cursor/cursor.obj");
    GenTextureMipmaps(&game.cursorModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture);
    game.planeModel = LoadModel("resources/models/planes/plane2/plane2.obj");
    GenTextureMipmaps(&game.planeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture);
    game.ballModels[0] = LoadModel("resources/models/balls/ball1/ball1.obj");
    GenTextureMipmaps(&game.ballModels[0].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture);
    game.ballModels[1] = LoadModel("resources/models/balls/ball2/ball2.obj");
    GenTextureMipmaps(&game.ballModels[1].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture);
    game.ballModels[2] = LoadModel("resources/models/balls/ball3/ball3.obj");
    GenTextureMipmaps(&game.ballModels[2].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture);
    game.ballModels[3] = LoadModel("resources/models/balls/ball4/ball4.obj");
    GenTextureMipmaps(&game.ballModels[3].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture);
    game.ballShotSounds[0] = LoadSound("resources/audio/sounds/cat/catChirp1.wav");
    game.ballShotSounds[1] = LoadSound("resources/audio/sounds/cat/catMeow1.wav");
    game.ballShotSounds[2] = LoadSound("resources/audio/sounds/cat/catMeow2.wav");
    game.ballShotSounds[3] = LoadSound("resources/audio/sounds/cat/catMeow3.wav");
    game.ballShotSounds[4] = LoadSound("resources/audio/sounds/cat/catMeow4.wav");

    initializeGameState(&game);

    return game;
}

void deinitializeGame(const Game *game)
{
    UnloadModel(game->trackModel);
    UnloadModel(game->cursorModel);
    UnloadModel(game->planeModel);
    UnloadModel(game->baseModel);

    for (size_t i = 0; i < BALL_MODEL_COUNT; i++)
    {
        UnloadModel(game->ballModels[i]);
    }

    for (size_t i = 0; i < BALL_SHOT_SOUND_SIZE; i++)
    {
        UnloadSound(game->ballShotSounds[i]);
    }

    CloseWindow();
}

static void antiAliasModelTextures(Game *game)
{
    SetTextureFilter(game->baseModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture, TEXTURE_FILTER_POINT);
    SetTextureFilter(game->cursorModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture, TEXTURE_FILTER_POINT);
    SetTextureFilter(game->trackModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture, TEXTURE_FILTER_POINT);

    for (size_t i = 0; i < game->ballCount; i++)
    {
        SetTextureFilter(game->balls[i].model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture, TEXTURE_FILTER_POINT);
    }

    for (size_t i = 0; i < SHOOTING_BALL_BUFFER_SIZE; i++)
    {
        SetTextureFilter(game->shootingBalls[i].model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture, TEXTURE_FILTER_POINT);
    }

    for (size_t i = 0; i < game->destroyingBallRangeCount; i++)
    {
        for (size_t j = 0; j < game->destroyingBallRanges[i].ballCount; j++)
        {
            SetTextureFilter(game->destroyingBallRanges[i].balls[j].model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture, TEXTURE_FILTER_POINT);
        }
    }
}

void processFrame(Game *game)
{
    /**
     * ************************************************************
     * USER INPUT ONLY (FIRST)
     * ************************************************************
     */
    Controls controls = processControls();

    UpdateCameraPro(&game->camera, controls.movement, Vector3Zero(), 0.0f);

    game->cursorPosition.x += controls.rotation.x;
    game->cursorPosition.y = 0.0f;
    game->cursorPosition.z += controls.rotation.y;
    game->cursorPosition = Vector3Clamp(game->cursorPosition, (Vector3){-40.0f, game->cursorPosition.y, -40.0f}, (Vector3){40.0f, game->cursorPosition.y, 40.0f});
    // https://math.stackexchange.com/questions/707673/find-angle-in-degrees-from-one-point-to-another-in-2d-space
    float baseRotationAngle = atan2f(game->cursorPosition.x - game->basePosition.x, game->cursorPosition.z - game->basePosition.z);
    float nextBallToBeShotX = cosf(baseRotationAngle) * (game->shootingBalls[0].position.x - game->basePosition.x) - sinf(baseRotationAngle) * (game->shootingBalls[0].position.z - game->basePosition.z) + game->basePosition.x;
    float nextBallToBeShotZ = -(sinf(baseRotationAngle) * (game->shootingBalls[0].position.x - game->basePosition.x) + cosf(baseRotationAngle) * (game->shootingBalls[0].position.z - game->basePosition.z) - game->basePosition.z);

    /**
     * Limit what the player can do when it's game over
     */
    if (!game->isGameOver)
    {
        /**
         * Handle mouse click to shoot ball
         */
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            for (size_t i = 2; i < SHOOTING_BALL_BUFFER_SIZE; i++)
            {
                if (!game->shootingBalls[i].isShooting)
                {
                    /**
                     * Play sound indicating ball was shot
                     */
                    size_t ballShotSoundIndex = GetRandomValue(0, BALL_SHOT_SOUND_SIZE - 1);
                    PlaySound(game->ballShotSounds[ballShotSoundIndex]);
                    /**
                     * Set first available non shooting ball (i > 1) to be the next ball to be shot
                     */
                    game->shootingBalls[i] = game->shootingBalls[0];
                    /**
                     * Set x and z position of first non shooting ball to start from the rotation set between
                     * the center of the base and current mouse position
                     */
                    game->shootingBalls[i].position.x = nextBallToBeShotX;
                    game->shootingBalls[i].position.z = nextBallToBeShotZ;
                    /**
                     * Set first available non shooting ball to start shooting
                     */
                    game->shootingBalls[i].isShooting = true;
                    /**
                     * Set target of where the ball should be shot at based on scaled mouse position.
                     *
                     * If the cursor position is exactly the same as the base position, the target
                     * position of the ball will become {0.0f, 0.0f, 120.0f}.
                     *
                     * Otherwise, move it 120.0f in the direction of the cursor position.
                     *
                     * With this condition in place, we prevent the shooting ball from becoming stuck
                     * at the base, when the cursor and base are positioned at the same point, since
                     * scaling a Vector3Zero() by any length will always result in a Vector3Zero().
                     *
                     * NOTE: The Vector3Zero() occurs from the Vector3Subtract() when the cursor and
                     * base positions are the same.
                     */
                    Vector3 shootingBallTarget = Vector3Subtract(game->cursorPosition, game->basePosition);

                    if (game->cursorPosition.x == game->basePosition.x && game->cursorPosition.z == game->basePosition.z)
                    {
                        game->shootingBalls[i].target = (Vector3){shootingBallTarget.x, shootingBallTarget.y, 120.0f};
                    }
                    else
                    {
                        Vector3 scaledShootingBallTarget = Vector3Scale(shootingBallTarget, 120.0f / Vector3Length(shootingBallTarget));
                        game->shootingBalls[i].target = scaledShootingBallTarget;
                    }
                    /**
                     * Get position of the next ball to be shot
                     */
                    Vector3 nextBallToBeShotPosition = game->shootingBalls[0].position;
                    /**
                     * The next ball to be shot becomes the ball after next ball to be shot
                     */
                    game->shootingBalls[0] = game->shootingBalls[1];
                    /**
                     * Set position of next ball to be shot to where the previous next ball to be shot was
                     */
                    game->shootingBalls[0].position = nextBallToBeShotPosition;
                    /**
                     * Get a random ball from the set of balls on the track
                     */
                    unsigned short int ballAfterNextToBeShotBallType = randomBallTypeFromBallOnTrackSet(game);
                    /**
                     * Set ball type and model of ball after next ball to be shot
                     */
                    game->shootingBalls[1].ballType = ballAfterNextToBeShotBallType;
                    game->shootingBalls[1].model = game->ballModels[ballAfterNextToBeShotBallType];

                    break;
                }
            }
        }
    }

    /**
     * ************************************************************
     * NON USER INPUT ONLY (SECOND)
     * ************************************************************
     */
    antiAliasModelTextures(game);

    for (size_t i = 0; i < game->destroyingBallRangeCount; i++)
    {
        for (size_t j = 0; j < game->destroyingBallRanges[i].ballCount; j++)
        {
            float distanceBetweenOriginAndDestroyingBall = Vector3Distance(Vector3Zero(), game->destroyingBallRanges[i].balls[j].position);

            if (distanceBetweenOriginAndDestroyingBall > DESTROYING_BALL_DEAD_ZONE)
            {
                for (size_t k = i + 1; k < game->destroyingBallRangeCount; k++)
                {
                    game->destroyingBallRanges[k - 1] = game->destroyingBallRanges[k];
                }

                game->destroyingBallRangeCount--;

                break;
            }
            else
            {
                Vector2 nextDestroyingBallPosition = Vector2MoveTowards((Vector2){game->destroyingBallRanges[i].balls[j].position.x, game->destroyingBallRanges[i].balls[j].position.z}, (Vector2){game->destroyingBallRanges[i].balls[j].target.x, game->destroyingBallRanges[i].balls[j].target.z}, 0.1f);
                Vector3 previousBallPoint = game->destroyingBallRanges[i].balls[j].position;
                Vector3 nextBallPoint = (Vector3){nextDestroyingBallPosition.x, game->destroyingBallRanges[i].balls[j].position.y, nextDestroyingBallPosition.y};
                game->destroyingBallRanges[i].balls[j].position = nextBallPoint;
                Vector3 ballRotationAxis = Vector3Zero();

                if (previousBallPoint.x > nextBallPoint.x)
                {
                    ballRotationAxis.z = 1.0f;
                }

                if (previousBallPoint.x < nextBallPoint.x)
                {
                    ballRotationAxis.z = -1.0f;
                }

                if (previousBallPoint.z > nextBallPoint.z)
                {
                    ballRotationAxis.x = -1.0f;
                }

                if (previousBallPoint.z < nextBallPoint.z)
                {
                    ballRotationAxis.x = 1.0f;
                }

                float angleRadians = distanceBetweenOriginAndDestroyingBall * PI / 4.0f;
                Matrix ballRotationMatrix = MatrixRotate(ballRotationAxis, angleRadians);
                Matrix ballTranslationMatrix = MatrixTranslate(nextDestroyingBallPosition.x, game->destroyingBallRanges[i].balls[j].position.y, nextDestroyingBallPosition.y);
                Matrix ballTransformationMatrix = MatrixMultiply(ballRotationMatrix, ballTranslationMatrix);
                game->destroyingBallRanges[i].balls[j].model.transform = ballTransformationMatrix;
            }
        }
    }

    for (size_t i = 2; i < SHOOTING_BALL_BUFFER_SIZE; i++)
    {
        if (game->shootingBalls[i].isShooting)
        {
            float distanceBetweenOriginAndShootingBall = Vector3Distance(Vector3Zero(), game->shootingBalls[i].position);

            if (distanceBetweenOriginAndShootingBall > 30.0f)
            {
                game->shootingBalls[i].isShooting = false;
            }
            else
            {
                bool isCollision = false;

                for (size_t j = 0; j < game->ballCount; j++)
                {
                    Vector3 pointOnTrack = ballTrackPointToPoint(game->track, game->balls[j]);
                    isCollision = CheckCollisionSpheres(game->shootingBalls[i].position, BALL_RADIUS, pointOnTrack, BALL_RADIUS);

                    if (isCollision)
                    {
                        size_t indexToInsertBallOnTrack = insertBallOnTrackAtPosition(game, game->shootingBalls[i], i, j);
                        removeConsecutiveTrackBallsAtPosition(game, indexToInsertBallOnTrack);

                        break;
                    }
                }

                if (!isCollision)
                {
                    Vector2 nextShootingBallPosition = Vector2MoveTowards((Vector2){game->shootingBalls[i].position.x, game->shootingBalls[i].position.z}, (Vector2){game->shootingBalls[i].target.x, game->shootingBalls[i].target.z}, 1.0f);
                    game->shootingBalls[i].position = (Vector3){nextShootingBallPosition.x, game->shootingBalls[i].position.y, nextShootingBallPosition.y};
                }
            }
        }
    }

    int headIndexOfLastGroupOfDisconnectedTrackBalls = findHeadIndexOfLastGroupOfDisconnectedTrackBalls(game);

    for (int i = 0; i < game->ballCount; i++)
    {
        float distanceToRollback = 0.0f;

        if (i < game->ballCount - 1)
        {
            for (int j = i + 1; j < game->ballCount; j++)
            {
                if (game->balls[j - 1].trackPoint - game->balls[j].trackPoint > game->ballSizeToTrackPoint && game->balls[j - 1].ballType == game->balls[j].ballType)
                {
                    float distanceToNextHeadFromCurrentBall = game->balls[i].trackPoint - game->balls[j].trackPoint;
                    float lengthOfBallsBeforeGap = game->ballSizeToTrackPoint + (game->ballSizeToTrackPoint * ((float)j - (float)i - 1.0f));

                    if (distanceToNextHeadFromCurrentBall > lengthOfBallsBeforeGap)
                    {
                        distanceToRollback -= distanceToNextHeadFromCurrentBall - lengthOfBallsBeforeGap;

                        break;
                    }
                }
            }
        }

        float distanceToRollForward = i < headIndexOfLastGroupOfDisconnectedTrackBalls ? 0.0f : game->options.speedMultiplier;

        roll(
            &game->balls[i],
            game->track,
            distanceToRollback < 0.0f ? Clamp(distanceToRollback + game->options.speedMultiplier, -5.0f, 0.0f) : distanceToRollForward);

        if (!game->balls[i].isBeforeEnd)
        {
            /**
             * When head track ball reaches the end of the track, it's game over.
             * Increase speed so all the track balls get to the end of the track faster
             * and disable shooting balls by setting game over flag.
             */
            if (!game->isGameOver)
            {
                game->isGameOver = true;
                game->options.speedMultiplier = 10.0f;
            }

            // Once the tail track ball reaches the end of the track, reinitialize game
            if (i == game->ballCount - 1)
            {
                initializeGameState(game);
            }
        }

        headIndexOfLastGroupOfDisconnectedTrackBalls = findHeadIndexOfLastGroupOfDisconnectedTrackBalls(game);
    }

    // reinitialize game state when level is beat
    if (game->ballCount == 0)
    {
        initializeGameState(game);
    }

    /**
     * Change ball type to one that is on the track for any shooting ball that is not currently shooting
     */
    for (size_t i = 0; i < SHOOTING_BALL_BUFFER_SIZE; i++)
    {
        bool isShootingBallOnTrack = isBallTypeOnTrack(game, game->shootingBalls[i].ballType);

        if (!game->shootingBalls[i].isShooting && !isShootingBallOnTrack)
        {
            initializeShootingBall(game, i);
        }
    }

    /**
     * ************************************************************
     * DRAWING ONLY (LAST)
     * ************************************************************
     */
    BeginDrawing();

    ClearBackground(BLACK);

    BeginMode3D(game->camera);

    // DrawGrid(100, 1.0f);
    DrawModel(game->cursorModel, game->cursorPosition, 5.0f, WHITE);
    DrawModel(game->trackModel, Vector3Zero(), 1.0f, WHITE);
    DrawModel(game->planeModel, (Vector3){0.0f, -0.6f, 0.0f}, 1.0f, WHITE);
    DrawModelEx(game->baseModel, game->basePosition, (Vector3){0.0f, 1.0f, 0.0f}, baseRotationAngle * RAD2DEG, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);

    for (size_t i = 0; i < game->destroyingBallRangeCount; i++)
    {
        for (size_t j = 0; j < game->destroyingBallRanges[i].ballCount; j++)
        {
            float distanceBetweenOriginAndDestroyingBall = Vector3Distance(Vector3Zero(), game->destroyingBallRanges[i].balls[j].position);
            Color ballColor = Fade(WHITE, 1.0f - (distanceBetweenOriginAndDestroyingBall * 1.42f / DESTROYING_BALL_DEAD_ZONE));

            DrawModel(game->destroyingBallRanges[i].balls[j].model, Vector3Zero(), 1.0f, ballColor);
        }
    }

    for (size_t i = 0; i < SHOOTING_BALL_BUFFER_SIZE; i++)
    {
        /**
         * i = 0 -> Next ball to be shot
         * i = 1 -> Ball after next to be shot
         * i > 1 -> possible balls that are still shooting
         */
        if (i == 0)
        {
            DrawModelEx(game->shootingBalls[0].model, (Vector3){nextBallToBeShotX, game->shootingBalls[0].position.y, nextBallToBeShotZ}, (Vector3){0.0f, 1.0f, 0.0f}, baseRotationAngle * RAD2DEG, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
        }

        if (i == 1)
        {
            DrawModelEx(game->shootingBalls[1].model, game->shootingBalls[1].position, (Vector3){0.0f, 1.0f, 0.0f}, baseRotationAngle * RAD2DEG, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
        }

        if (i > 1 && game->shootingBalls[i].isShooting)
        {
            DrawModel(game->shootingBalls[i].model, game->shootingBalls[i].position, 1.0f, WHITE);
        }
    }

    for (size_t i = 0; i < game->ballCount; i++)
    {
        if (game->balls[i].shouldDrawBall)
        {
            DrawModel(game->balls[i].model, Vector3Zero(), 1.0f, WHITE);
        }
    }

#ifdef DEBUG
    // Draw X-axis (red)
    DrawLine3D((Vector3){-10.0f, 0.0f, 0.0f}, (Vector3){10.0f, 0.0f, 0.0f}, YELLOW);
    // Draw Y-axis (green)
    DrawLine3D((Vector3){0.0f, -10.0f, 0.0f}, (Vector3){0.0f, 10.0f, 0.0f}, GREEN);
    // Draw Z-axis (blue)
    DrawLine3D((Vector3){0.0f, 0.0f, -10.0f}, (Vector3){0.0f, 0.0f, 10.0f}, BLUE);

    for (size_t i = 0; i < game->track.numberOfPoints - 1; i++)
    {
        DrawLine3D(game->track.points[i], game->track.points[i + 1], RED);
    }
#endif // DEBUG

    EndMode3D();

#ifdef DEBUG
    DrawText(TextFormat("Camera position: [%i, %i, %i]", (int)game->camera.position.x, (int)game->camera.position.y, (int)game->camera.position.z), 10, 10, 20, LIME);
    DrawText(TextFormat("Camera target: [%i, %i, %i]", (int)game->camera.target.x, (int)game->camera.target.y, (int)game->camera.target.z), 10, 40, 20, LIME);
    DrawText(TextFormat("Camera up: [%i, %i, %i]", (int)game->camera.up.x, (int)game->camera.up.y, (int)game->camera.up.z), 10, 70, 20, LIME);
    DrawText(TextFormat("Cursor position: [%i, %i, %i]", (int)game->cursorPosition.x, (int)game->cursorPosition.y, (int)game->cursorPosition.z), 10, 100, 20, LIME);
    DrawText(TextFormat("Base rotation angle: [%f]", baseRotationAngle), 10, 130, 20, LIME);
#endif // DEBUG

    EndDrawing();
}