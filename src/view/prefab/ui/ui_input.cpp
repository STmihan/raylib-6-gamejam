#include "view/prefab/ui/ui_input.h"

namespace view::ui
{
void UiInput::BeginFrame()
{
    mouse_ = GetMousePosition();
    down_ = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    pressed_ = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    released_ = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    blockers_.clear();
}

bool UiInput::CapturesMouse() const
{
    for (const Rectangle& r : blockers_)
    {
        if (CheckCollisionPointRec(mouse_, r)) return true;
    }
    return false;
}
}
