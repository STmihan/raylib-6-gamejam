#include "app/core/app.h"

#include "data/space/hex.h"
#include "data/time/time_config.h"
#include "data/space/world_config.h"
#include "view/debug/debug_ui.h"

namespace app
{
void InitApp(App& app)
{
    app.renderer.Init();
    app.cameraRig.Init();
    app.accumulator = 0.0;
    app.map = logic::BuildMap();
    app.simulation.Init(app.currentState);
    app.previousState = app.currentState;
}

void StepApp(App& app)
{
    app.accumulator += GetFrameTime();

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

    app.cameraRig.Update();

    auto alpha = static_cast<float>(app.accumulator / data::TickDelta);
    auto overlay = [&app]() { view::DrawDebugOverlay(app.cameraRig.BoundsRadiusRef(), app.renderer.Water()); };
    app.renderer.Draw(app.previousState, app.currentState, alpha, app.cameraRig.Camera(), app.map, overlay);
}

void ShutdownApp(App& app)
{
    app.renderer.Shutdown();
}
}
