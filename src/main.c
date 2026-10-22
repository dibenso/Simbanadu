#include <stdlib.h>
#include "raylib.h"
#include "game.h"

int main(void)
{
    Game game = initializeGame();

    while (!WindowShouldClose())
    {
        processFrame(&game);
    }

    deinitializeGame(&game);

    return EXIT_SUCCESS;
}