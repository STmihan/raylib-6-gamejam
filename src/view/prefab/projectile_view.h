#ifndef VIEW_PREFAB_PROJECTILE_VIEW_H
#define VIEW_PREFAB_PROJECTILE_VIEW_H

#include "logic/state/game_state.h"
#include "view/prefab/plane_orbit.h"

namespace view
{
class ModelRegistry;
class MuzzleRegistry;

class ProjectileView
{
public:
    void Draw(const ModelRegistry& models, const MuzzleRegistry& muzzles, const logic::GameState& previous,
              const logic::GameState& current, float alpha, const PlaneOrbitParams& orbit, float time) const;
    void DrawMuzzleGizmos(const ModelRegistry& models, const MuzzleRegistry& muzzles,
                          const logic::GameState& previous, const logic::GameState& current, float alpha,
                          const PlaneOrbitParams& orbit) const;

    float& ArcHeightRef() { return arcHeight_; }
    float& ArcCurveRef() { return arcCurve_; }
    float& ShellScaleRef() { return shellScale_; }
    float& LaunchHeightRef() { return launchHeight_; }
    float& TargetHeightRef() { return targetHeight_; }

private:
    float arcHeight_ = 1.2f;
    float arcCurve_ = 1.5f;
    float shellScale_ = 1.0f;
    float launchHeight_ = 0.8f;
    float targetHeight_ = 0.9f;
};
}

#endif
