#ifndef VIEW_INTERPOLATOR_H
#define VIEW_INTERPOLATOR_H

#include "data/vec.h"
#include "logic/game_state.h"

namespace view {

inline data::Vec2 InterpolatedPosition(const logic::Entity &previous, const logic::Entity &current, float alpha) {
    data::Vec2 from = previous.active ? previous.position : current.position;
    return data::Lerp(from, current.position, alpha);
}

}

#endif
