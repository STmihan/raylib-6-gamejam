#ifndef DEBUG_AIR_TEST_SCENE_H
#define DEBUG_AIR_TEST_SCENE_H

#include "raylib.h"

#include "logic/state/game_state.h"

namespace debug
{
class AirTestScene
{
public:
    bool Active() const { return active_; }
    bool& ActiveRef() { return active_; }

    int& PlaneCountRef() { return planeCount_; }
    int& ModeRef() { return mode_; }

    void UpdateCamera();
    Camera3D Camera() const;
    const logic::GameState& State();

private:
    void Rebuild();

    bool active_ = false;
    int planeCount_ = 2;
    int mode_ = 0;
    int builtCount_ = -1;
    int builtMode_ = -1;

    float yaw_ = 45.0f;
    float pitch_ = 30.0f;
    float distance_ = 8.0f;

    logic::GameState state_{};
};
}

#endif
