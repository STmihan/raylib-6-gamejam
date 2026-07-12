#include "app/core/app.h"

#include <random>

#include "ai/policy.h"
#include "app/input/cursor.h"
#include "audio/sound.h"
#include "data/economy/economy.h"
#include "data/space/hex.h"
#include "data/space/world_config.h"
#include "data/time/time_config.h"
#include "debug/debug_input.h"
#include "debug/debug_ui.h"
#include "logic/cards/cards.h"

namespace app
{
namespace
{
void ProcessEnemyAI(App& app)
{
    if (app.currentState.winner >= 0) return;
    std::uint64_t cooldown = static_cast<std::uint64_t>(data::AiDeployCooldownSeconds() * data::TickRate);
    if (cooldown < 1) cooldown = 1;
    if (app.currentState.tick - app.lastAiTick < cooldown) return;
    app.lastAiTick = app.currentState.tick;

    ai::Action action = ai::DecideAction(app.currentState, app.map, data::EnemyTeam);
    int spawned = ai::ApplyAction(app.currentState, app.map, data::EnemyTeam, action);
    if (spawned >= 0) app.previousState.entities[spawned] = app.currentState.entities[spawned];
    ai::RunMicro(app.currentState, app.map, data::EnemyTeam);
}

void ProcessDeploy(App& app, Camera3D camera)
{
    int slot;
    Vector2 screenPos;
    if (!app.renderer.Hand().TakeDrop(slot, screenPos)) return;

    data::Vec2 logicPos;
    if (!CursorLogic(camera, screenPos, logicPos)) return;
    data::Offset cell = data::CellFromLogic(logicPos);

    int spawned = logic::PlayCard(app.currentState, app.map, data::PlayerTeam, slot, cell.col, cell.row);
    if (spawned < 0) return;
    app.previousState.entities[spawned] = app.currentState.entities[spawned];
    audio::Play("place-unit");
}

void ProcessMerge(App& app)
{
    int host;
    int donor;
    if (!app.renderer.Hand().TakeMerge(host, donor)) return;
    logic::MergeSlots(app.currentState, data::PlayerTeam, host, donor);
    audio::Play("button-click");
}
}

std::uint32_t NewSeed()
{
    std::random_device rng;
    std::uint32_t seed = rng();
    return seed == 0 ? 0x9e3779b9U : seed;
}

void InitApp(App& app)
{
    audio::Init();
    audio::PlayMusic();
    app.map = logic::BuildMap();
    app.renderer.Init(app.map);
    app.cameraRig.Init();
    app.accumulator = 0.0;
    app.lastAiTick = 0;
    app.simulation.Init(app.currentState, app.map, NewSeed());
    app.previousState = app.currentState;
}

void StepApp(App& app)
{
    audio::Update();

    bool paused = app.renderer.HudPaused();
    float speed = 1.0f;
#if defined(DEBUG_BUILD)
    static debug::Cheats cheats;
    cheats.Update(app);
    paused = paused || cheats.Paused();
    speed = cheats.Speed();
#endif

    if (app.currentState.winner >= 0 && IsKeyPressed(KEY_R))
    {
        app.simulation.Init(app.currentState, app.map, NewSeed());
        app.previousState = app.currentState;
        app.accumulator = 0.0;
        app.lastAiTick = 0;
    }

    app.renderer.Ui().BeginFrame();
    app.renderer.Controls().Update(app.renderer.Ui(), GetFrameTime());
    app.renderer.Hand().Update(app.renderer.Ui().Input(), GetFrameTime(), app.currentState);

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
        ProcessEnemyAI(app);
    }

    app.audioEvents.Observe(app.currentState);
    audio::SetMusicOvertime(static_cast<int>(app.currentState.tick / data::TickRate) >=
                            data::MatchDurationSeconds());

    app.cameraRig.Update();

    Camera3D camera = app.cameraRig.Camera();
#if defined(DEBUG_BUILD)
    cheats.ApplyFreeCamera(camera, GetFrameTime());
#endif

    ProcessMerge(app);
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
    audio::Shutdown();
}
}
