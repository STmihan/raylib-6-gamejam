#ifndef APP_APP_H
#define APP_APP_H

#include "raylib.h"

#include "logic/game_state.h"
#include "logic/map.h"
#include "logic/simulation.h"
#include "view/camera_rig.h"
#include "view/renderer.h"

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
};

void InitApp(App& app);
void StepApp(App& app);
void ShutdownApp(App& app);
}

#endif
