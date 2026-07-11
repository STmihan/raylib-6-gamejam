#ifndef APP_INPUT_UNIT_CONTROL_H
#define APP_INPUT_UNIT_CONTROL_H

#include "raylib.h"

#include "data/space/vec.h"

namespace app
{
struct App;

class UnitControl
{
public:
    void Update(App& app, Camera3D camera);

private:
    void Acquire(App& app, data::Vec2 logicPos, bool onField);
    void Command(App& app, int slot, data::Vec2 logicPos, bool onField);
    void FeedRenderer(App& app, Camera3D camera);

    int dragSlot_ = -1;
};
}

#endif
