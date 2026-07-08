#include "app/app.h"

#include "data/hex.h"
#include "data/time_config.h"
#include "data/world_config.h"

namespace app {

namespace {

Camera3D BuildCamera() {
    data::Vec2 center = data::FieldCenterLogic();
    Vector3 focus = { center.x * data::RenderScale, 0.0f, center.y * data::RenderScale };

    Camera3D camera = {};
    camera.position = { focus.x, 28.0f, focus.z + 22.0f };
    camera.target = focus;
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    return camera;
}

}

void InitApp(App &app) {
    app.renderer.Init();
    app.camera = BuildCamera();
    app.accumulator = 0.0;
    app.map = logic::BuildMap();
    app.simulation.Init(app.currentState);
    app.previousState = app.currentState;
}

void StepApp(App &app) {
    app.accumulator += GetFrameTime();

    int steps = 0;
    while (app.accumulator >= data::TickDelta && steps < data::MaxTicksPerFrame) {
        app.previousState = app.currentState;
        app.simulation.Step(app.currentState, static_cast<float>(data::TickDelta));
        app.accumulator -= data::TickDelta;
        steps++;
    }
    if (steps == data::MaxTicksPerFrame) {
        app.accumulator = 0.0;
    }

    float alpha = static_cast<float>(app.accumulator / data::TickDelta);
    app.renderer.Draw(app.previousState, app.currentState, alpha, app.camera, app.map);
}

void ShutdownApp(App &app) {
    app.renderer.Shutdown();
}

}
