#ifndef VIEW_PREFAB_DEPLOY_RING_H
#define VIEW_PREFAB_DEPLOY_RING_H

#include "raylib.h"

#include "logic/state/game_state.h"

namespace view
{
class DeployRingView
{
public:
    void Load(Shader ring, Texture2D white);

    void Draw(Camera3D camera, const logic::GameState& previous, const logic::GameState& current,
              float alpha) const;

private:
    Shader ring_{};
    Texture2D white_{};
    int locProgress_ = 0;
    bool loaded_ = false;
};
}

#endif
