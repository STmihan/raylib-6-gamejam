#ifndef DEBUG_INPUT_H
#define DEBUG_INPUT_H

#include "raylib.h"

#include "debug/free_camera.h"

namespace app { struct App; }

namespace debug
{
class Cheats
{
public:
    void Update(app::App& app);
    void ApplyFreeCamera(Camera3D& camera, float dt);

    bool Paused() const { return paused_; }
    float Speed() const { return speed_; }

private:
    bool paused_ = false;
    float speed_ = 1.0f;
    FreeCamera freeCamera_;
    bool freeFly_ = false;
};
}

#endif
