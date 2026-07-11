#include "app/core/app.h"

#include "app/input/cursor.h"
#include "data/card/card.h"
#include "data/economy/economy.h"
#include "data/space/hex.h"
#include "data/space/world_config.h"
#include "data/time/time_config.h"
#include "debug/debug_input.h"
#include "debug/debug_ui.h"
#include "logic/deploy.h"

namespace app
{
namespace
{
void ProcessDeploy(App& app, Camera3D camera)
{
    int slot;
    data::UnitType type;
    Vector2 screenPos;
    if (!app.renderer.Hand().TakeDrop(slot, type, screenPos)) return;

    data::Vec2 logicPos;
    if (!CursorLogic(camera, screenPos, logicPos)) return;
    data::Offset cell = data::CellFromLogic(logicPos);
    if (!logic::IsDeployable(app.map, cell.col, cell.row, data::PlayerTeam, type)) return;
    if (logic::CellHasUnit(app.currentState, cell.col, cell.row)) return;

    int cost = data::CardDefOf(type).cost;
    int pidx = data::TeamIndex(data::PlayerTeam);
    if (app.currentState.resource[pidx] < static_cast<float>(cost)) return;

    int spawned = logic::Simulation::Deploy(app.currentState, type, data::PlayerTeam, cell.col, cell.row);
    if (spawned < 0) return;
    app.previousState.entities[spawned] = app.currentState.entities[spawned];
    app.currentState.resource[pidx] -= static_cast<float>(cost);
    app.renderer.Hand().MarkPlayed(slot);
}
}
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

    if (app.currentState.winner >= 0 && IsKeyPressed(KEY_R))
    {
        app.simulation.Init(app.currentState, app.map);
        app.previousState = app.currentState;
        app.accumulator = 0.0;
    }

    app.renderer.Ui().BeginFrame();
    app.renderer.Hand().Update(app.renderer.Ui().Input(), GetFrameTime());

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

    ProcessDeploy(app, camera);
    app.unitControl.Update(app, camera);

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
