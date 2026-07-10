#include "app/core/app.h"

#include "data/time/time_config.h"
#include "debug/debug_input.h"
#include "debug/debug_ui.h"

namespace app
{
void InitApp(App& app)
{
    app.renderer.Init();
    app.cameraRig.Init();
    app.accumulator = 0.0;
    app.map = logic::BuildMap();
    app.simulation.Init(app.currentState, app.map);
    app.previousState = app.currentState;
}

void StepApp(App& app)
{
    bool paused = false;
    float speed = 1.0f;
#if defined(DEBUG_BUILD)
    static debug::Cheats cheats;
    cheats.Update(app);
    paused = cheats.Paused();
    speed = cheats.Speed();
#endif

    float delta = GetFrameTime() * speed;

    static double viewClock = 0.0;
    if (!paused) viewClock += delta;

    if (!paused)
    {
        app.accumulator += delta;

        int steps = 0;
        while (app.accumulator >= data::TickDelta && steps < data::MaxTicksPerFrame)
        {
            app.previousState = app.currentState;
            app.simulation.Step(app.currentState, data::TickDelta);
            app.accumulator -= data::TickDelta;
            steps++;
        }
        if (steps == data::MaxTicksPerFrame)
        {
            app.accumulator = 0.0;
        }
    }

    app.cameraRig.Update();

    Camera3D camera = app.cameraRig.Camera();
#if defined(DEBUG_BUILD)
    cheats.ApplyFreeCamera(camera, GetFrameTime());
#endif

    auto alpha = static_cast<float>(app.accumulator / data::TickDelta);
    auto overlay = [&app]() {
        debug::DrawDebugOverlay(app.cameraRig.BoundsRadiusRef(), app.renderer, [&app] {
            app.currentState.resource = {0.0f, 0.0f};
            app.previousState.resource = {0.0f, 0.0f};
        });
    };
    app.renderer.Draw(app.previousState, app.currentState, alpha, static_cast<float>(viewClock), camera, app.map,
                      overlay);
}

void ShutdownApp(App& app)
{
    app.renderer.Shutdown();
}
}
