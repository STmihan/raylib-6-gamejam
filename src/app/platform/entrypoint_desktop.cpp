#include "app/core/entrypoint.h"

#if !defined(__EMSCRIPTEN__)

#include "raylib.h"

#include "app/core/app.h"
#include "view/debug/debug_ui.h"

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 720
#endif

#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 720
#endif

namespace app
{
int RunDesktop()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib-6-gamejam");
    ChangeDirectory(GetApplicationDirectory());
    SetTargetFPS(60);
    view::DebugUiSetup();

    App app{};
    InitApp(app);

    while (!WindowShouldClose())
    {
        StepApp(app);
    }

    ShutdownApp(app);
    view::DebugUiShutdown();
    CloseWindow();
    return 0;
}
}

#else

namespace app
{
int RunDesktop()
{
    return 0;
}
}

#endif
