#ifndef VIEW_PREFAB_HP_BAR_H
#define VIEW_PREFAB_HP_BAR_H

#include "raylib.h"

#include "logic/state/game_state.h"

namespace view
{
class HpBarView
{
public:
    void Load();
    void Unload();

    void DrawEntities(Camera3D camera, const logic::GameState& previous, const logic::GameState& current,
                      float alpha) const;
    void DrawSingle(Camera3D camera, Vector3 worldTop, float fraction, Color fill) const;

private:
    void Bar(Vector2 topCenter, float fraction, Color fill) const;

    Texture2D white_{};
    bool loaded_ = false;
};
}

#endif
