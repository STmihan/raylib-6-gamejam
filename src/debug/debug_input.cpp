#include "debug/debug_input.h"

#include "app/core/app.h"

namespace debug
{
void Cheats::Update(app::App& app)
{
    if (IsKeyPressed(KEY_R))
    {
        app.simulation.Init(app.currentState, app.map);
        app.previousState = app.currentState;
        app.accumulator = 0.0;
    }

    if (IsKeyPressed(KEY_P)) paused_ = !paused_;

    if (IsKeyPressed(KEY_KP_1)) speed_ = 1.0f;
    if (IsKeyPressed(KEY_KP_2)) speed_ = 2.0f;
    if (IsKeyPressed(KEY_KP_3)) speed_ = 3.0f;
}

void Cheats::ApplyFreeCamera(Camera3D& camera, float dt)
{
    if (IsKeyPressed(KEY_L))
    {
        freeFly_ = !freeFly_;
        if (freeFly_)
        {
            freeCamera_.Sync(camera);
            DisableCursor();
        }
        else
        {
            EnableCursor();
        }
    }
    if (freeFly_)
    {
        freeCamera_.Update(dt);
        camera = freeCamera_.Camera();
    }
}
}
