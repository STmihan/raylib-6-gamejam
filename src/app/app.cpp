#include "app/app.h"

#include "data/hex.h"
#include "data/time_config.h"
#include "data/world_config.h"
#include "view/debug_ui.h"

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
        app.simulation.Step(app.currentState, static_cast<float>(data::TickDelta));
        app.accumulator -= data::TickDelta;
        steps++;
    }
    if (steps == data::MaxTicksPerFrame)
    {
        app.accumulator = 0.0;
    }

    app.cameraRig.Update();

    auto alpha = static_cast<float>(app.accumulator / data::TickDelta);
    auto overlay = [&app]() { view::DrawDebugOverlay(app.renderer.WaterParamsRef()); };
    app.renderer.Draw(app.previousState, app.currentState, alpha, app.cameraRig.Camera(), app.map, overlay);
}

void ShutdownApp(App& app)
{
    app.renderer.Shutdown();
}
}
