#include "app/core/entrypoint.h"

#if defined(__EMSCRIPTEN__)

#include <emscripten/emscripten.h>

#include "raylib.h"

#include "app/core/app.h"

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 720
#endif

#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 720
#endif

namespace app {

namespace {

App *g_app = nullptr;

void FrameTrampoline() {
    StepApp(*g_app);
}

}

int RunWeb() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib-6-gamejam");

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
