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

static void UpdateDrawFrame(void);

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib-game");

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

    CloseWindow();
    return 0;
}

static void UpdateDrawFrame(void)
{
    const char *message = "raylib is running at 720x720";
    int fontSize = 20;
    int messageWidth = MeasureText(message, fontSize);
    int messageX = (SCREEN_WIDTH - messageWidth)/2;
    int messageY = SCREEN_HEIGHT/2 - fontSize;

    BeginDrawing();

        ClearBackground(RAYWHITE);
        DrawText(message, messageX, messageY, fontSize, DARKGRAY);
        DrawFPS(10, 10);

    EndDrawing();
}
