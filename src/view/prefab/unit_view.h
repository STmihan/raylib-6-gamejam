#ifndef VIEW_PREFAB_UNIT_VIEW_H
#define VIEW_PREFAB_UNIT_VIEW_H

#include <array>

#include "data/sim/sim_config.h"
#include "logic/state/game_state.h"
#include "view/prefab/plane_orbit.h"

namespace view
{
class ModelRegistry;

class UnitView
{
public:
    void UpdateFlash(const logic::GameState& current, float dt);
    void TriggerFlashAll();
    void Draw(const ModelRegistry& models, const logic::GameState& previous, const logic::GameState& current,
              float alpha, bool includeDecals, const PlaneOrbitParams& orbit, float time) const;
    void DrawPreview(const ModelRegistry& models, data::UnitType type, data::Vec2 logic, data::Team team,
                     const PlaneOrbitParams& orbit, float time, Color tint) const;
    void DrawHighlight(const ModelRegistry& models, const logic::GameState& previous,
                       const logic::GameState& current, float alpha, const PlaneOrbitParams& orbit, float time,
                       int index, Color tint) const;

    float& BlobRadiusRef() { return blobRadius_; }
    float& BlobOpacityRef() { return blobOpacity_; }

private:
    std::array<float, data::MaxEntities> flashTimer_{};
    std::array<int, data::MaxEntities> lastHp_{};
    std::array<float, data::MaxEntities> combatBlend_{};
    float blobRadius_ = 1.2f;
    float blobOpacity_ = 0.5f;
};
}

#endif
