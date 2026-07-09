#ifndef DEBUG_PROJECTILE_TEST_SCENE_H
#define DEBUG_PROJECTILE_TEST_SCENE_H

#include "raylib.h"

#include "logic/state/game_state.h"

namespace debug
{
class ProjectileTestScene
{
public:
    bool Active() const { return active_; }
    bool& ActiveRef() { return active_; }

    int& AttackerRef() { return attacker_; }
    int& TargetRef() { return target_; }
    int AttackerCount() const;
    int TargetCount() const;
    const char* AttackerName(int index) const;
    const char* TargetName(int index) const;

    void UpdateCamera();
    void Update(float dt);
    Camera3D Camera() const;

    const logic::GameState& Previous() const { return previous_; }
    const logic::GameState& Current() const { return current_; }
    float Alpha() const { return alpha_; }

private:
    void Rebuild();
    void Step();

    bool active_ = false;
    int attacker_ = 0;
    int target_ = 5;
    int builtAttacker_ = -1;
    int builtTarget_ = -1;

    float yaw_ = 45.0f;
    float pitch_ = 25.0f;
    float distance_ = 10.0f;

    double accumulator_ = 0.0;
    float alpha_ = 0.0f;
    logic::GameState previous_{};
    logic::GameState current_{};
};
}

#endif
