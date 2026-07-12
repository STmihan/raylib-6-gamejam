#ifndef VIEW_PREFAB_UI_UI_INPUT_H
#define VIEW_PREFAB_UI_UI_INPUT_H

#include <vector>

#include "raylib.h"

namespace view::ui
{
class UiInput
{
public:
    void BeginFrame();

    void Block(Rectangle r) { blockers_.push_back(r); }
    bool CapturesMouse() const;

    bool Hover(Rectangle r) const { return CheckCollisionPointRec(mouse_, r); }
    bool Pressed(Rectangle r) const { return pressed_ && Hover(r); }
    bool Pressed() const { return pressed_; }
    bool Released() const { return released_; }
    Vector2 Mouse() const { return mouse_; }
    bool Down() const { return down_; }

private:
    Vector2 mouse_{};
    bool down_ = false;
    bool pressed_ = false;
    bool released_ = false;
    std::vector<Rectangle> blockers_;
};
}

#endif
