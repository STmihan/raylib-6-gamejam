#ifndef VIEW_ANIM_ANIM_CONTROLLER_H
#define VIEW_ANIM_ANIM_CONTROLLER_H

#include <array>

#include "data/sim/sim_config.h"
#include "logic/state/game_state.h"
#include "view/anim/anim_state.h"

namespace view
{
class AnimController
{
public:
    void Update(const logic::GameState& previous, const logic::GameState& current);

private:
    std::array<AnimState, data::MaxEntities> state_{};
    std::array<bool, data::MaxEntities> tracked_{};
    std::array<bool, data::MaxEntities> active_{};
};
}

#endif
