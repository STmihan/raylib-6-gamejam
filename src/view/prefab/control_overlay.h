#ifndef VIEW_PREFAB_CONTROL_OVERLAY_H
#define VIEW_PREFAB_CONTROL_OVERLAY_H

#include "data/space/vec.h"
#include "view/prefab/plane_orbit.h"

namespace logic { struct GameState; struct Map; }

namespace view
{
class ModelRegistry;
class UnitView;

class ControlOverlayView
{
public:
    void SetDrag(bool active, data::Vec2 from, data::Vec2 to, bool movable);
    void Draw(const logic::GameState& previous, const logic::GameState& current, float alpha, float time,
              const logic::Map& map, const ModelRegistry& models, const UnitView& units,
              const PlaneOrbitParams& orbit, const bool* occluded) const;

private:
    void DrawEnemyHighlight(const logic::GameState& previous, const logic::GameState& current, float alpha,
                            float time, const logic::Map& map, const ModelRegistry& models, const UnitView& units,
                            const PlaneOrbitParams& orbit, int target) const;
    void DrawMoveCell(const logic::Map& map, const ModelRegistry& models, const bool* occluded) const;

    bool active_ = false;
    bool movable_ = false;
    data::Vec2 from_{};
    data::Vec2 to_{};
};
}

#endif
