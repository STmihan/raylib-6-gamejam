#ifndef VIEW_RENDERER_H
#define VIEW_RENDERER_H

#include "raylib.h"

#include "logic/game_state.h"
#include "logic/map.h"

namespace view {

class Renderer {
public:
    void Init();
    void Shutdown();
    void Draw(const logic::GameState &previous, const logic::GameState &current, float alpha, Camera3D camera, const logic::Map &map);
};

}

#endif
