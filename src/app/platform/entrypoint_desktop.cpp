#include "app/core/entrypoint.h"

#if !defined(__EMSCRIPTEN__)

#include "raylib.h"

#include "assets/pack.h"

#include "app/core/app.h"
#include "app/platform/window_chrome.h"
#include "data/app/app_config.h"
#include "data/balance/balance.h"
#include "debug/debug_ui.h"

namespace app
{
int RunDesktop()
{
    SetConfigFlags(FLAG_WINDOW_UNDECORATED); // borderless: we draw our own title bar
    InitWindow(data::ScreenWidth, data::ScreenHeight, data::WindowTitle);
    ChangeDirectory(GetApplicationDirectory());
    assets::MountPack();

    Image icon = LoadImage("assets/icons/ui/icon.png");
    if (icon.data != nullptr)
    {
        SetWindowIcon(icon);
        UnloadImage(icon);
    }

    SetTargetFPS(data::TargetFps);
    debug::DebugUiSetup();

    char* config = LoadFileText("assets/config.json");
    data::LoadRulesFromString(config);
    UnloadFileText(config);

    App app{};
    InitApp(app);

    while (!WindowShouldClose() && !platform::CloseRequested())
    {
        StepApp(app);
    }

    ShutdownApp(app);
    debug::DebugUiShutdown();
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
