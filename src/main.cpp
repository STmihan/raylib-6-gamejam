#include "raylib.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 720
#endif

#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 720
#endif

static Texture2D grassTile;

static void UpdateDrawFrame();

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib-game");

#if !defined(__EMSCRIPTEN__)
    ChangeDirectory(GetApplicationDirectory());
#endif

    grassTile = LoadTexture("assets/hex_grass.png");

#if defined(__EMSCRIPTEN__)
    int browserManagedLoop = 1;
    int useRequestAnimationFrameTiming = 0;
    emscripten_set_main_loop(UpdateDrawFrame, useRequestAnimationFrameTiming, browserManagedLoop);
#else
    int targetFps = 60;
    SetTargetFPS(targetFps);

    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif

    UnloadTexture(grassTile);
    CloseWindow();
    return 0;
}

static void UpdateDrawFrame()
{
    int tileX = (SCREEN_WIDTH - grassTile.width)/2;
    int tileY = (SCREEN_HEIGHT - grassTile.height)/2;

    const char *message = "TESTING";
    int fontSize = 20;
    int messageWidth = MeasureText(message, fontSize);
    int messageX = (SCREEN_WIDTH - messageWidth)/2;
    int messageY = tileY + grassTile.height + 24;

    BeginDrawing();

        ClearBackground(RAYWHITE);
        DrawTexture(grassTile, tileX, tileY, WHITE);
        DrawText(message, messageX, messageY, fontSize, DARKGRAY);
        DrawFPS(10, 10);

    EndDrawing();
}
