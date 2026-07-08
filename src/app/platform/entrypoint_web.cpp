#include "app/core/entrypoint.h"

#if defined(__EMSCRIPTEN__)

#include <emscripten/emscripten.h>

#include "raylib.h"

#include "app/core/app.h"
#include "data/app/app_config.h"

namespace app {

namespace {

App *g_app = nullptr;

void FrameTrampoline() {
    StepApp(*g_app);
}

}

int RunWeb() {
    InitWindow(data::ScreenWidth, data::ScreenHeight, data::WindowTitle);

    static App app;
    g_app = &app;
    InitApp(app);

    emscripten_set_main_loop(FrameTrampoline, 0, 1);
    return 0;
}

}

#else

namespace app {

int RunWeb() {
    return 0;
}

}

#endif
