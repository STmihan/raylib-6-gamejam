#ifndef VIEW_PREFAB_UI_HUD_CONTROLS_H
#define VIEW_PREFAB_UI_HUD_CONTROLS_H

#include "raylib.h"

#include "view/prefab/ui/ui_context.h"

namespace view::ui
{
class HudControls
{
public:
    void Update(UiContext& ui, float dt);
    void Draw(UiContext& ui);

    bool Paused() const { return paused_; }

private:
    struct Layout
    {
        Rectangle pause;
        Rectangle sound;
        Rectangle panel;
        Rectangle track;
    };

    Layout Compute() const;

    bool paused_ = false;
    float volume_ = 1.0f;
    float lastVolume_ = 1.0f;
    float panelOpen_ = 0.0f;
    bool draggingVolume_ = false;
};
}

#endif
