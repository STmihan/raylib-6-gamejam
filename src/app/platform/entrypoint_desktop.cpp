#include "app/core/entrypoint.h"

#if !defined(__EMSCRIPTEN__)

#include "raylib.h"

#include "app/core/app.h"
#include "data/app/app_config.h"
#include "view/debug/debug_ui.h"

namespace app
{
int RunDesktop()
{
    InitWindow(data::ScreenWidth, data::ScreenHeight, data::WindowTitle);
    ChangeDirectory(GetApplicationDirectory());
    SetTargetFPS(data::TargetFps);
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
