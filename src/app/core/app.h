#ifndef APP_APP_H
#define APP_APP_H

#include <cstdint>

#include "raylib.h"

#include "app/core/audio_events.h"
#include "app/input/unit_control.h"
#include "logic/state/game_state.h"
#include "logic/world/map.h"
#include "logic/sim/simulation.h"
#include "view/camera/camera_rig.h"
#include "view/render/renderer.h"

namespace app
{
struct App
{
    logic::Simulation simulation;
    logic::GameState previousState;
    logic::GameState currentState;
    logic::Map map;
    view::Renderer renderer;
    view::CameraRig cameraRig;
    double accumulator;
    std::uint64_t lastAiTick;
    UnitControl unitControl;
    AudioEvents audioEvents;
};

std::uint32_t NewSeed();
void InitApp(App& app);
void StepApp(App& app);
void ShutdownApp(App& app);
}

#endif
