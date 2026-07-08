#ifndef APP_APP_H
#define APP_APP_H

#include "raylib.h"

#include "logic/game_state.h"
#include "logic/simulation.h"
#include "view/renderer.h"

namespace app {

struct App {
    logic::Simulation simulation;
    logic::GameState previousState;
    logic::GameState currentState;
    view::Renderer renderer;
    Camera3D camera;
    double accumulator;
};

void InitApp(App &app);
void StepApp(App &app);
void ShutdownApp(App &app);

}

#endif
