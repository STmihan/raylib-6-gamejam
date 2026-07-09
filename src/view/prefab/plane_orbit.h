#ifndef VIEW_PREFAB_PLANE_ORBIT_H
#define VIEW_PREFAB_PLANE_ORBIT_H

#include "raylib.h"

#include "logic/state/game_state.h"

namespace view
{
struct PlaneOrbitParams
{
    float airRadius = 2.0f;
    float groundRadius = 2.0f;
    float speed = 1.5f;
    float bank = -31.0f;
    float altitude = 1.6f;
};

Vector3 PlaneVisualPos(const logic::GameState& previous, const logic::GameState& current, int slot, float alpha,
                       float time, const PlaneOrbitParams& params, float& yawOut, float& rollOut);
}

#endif
