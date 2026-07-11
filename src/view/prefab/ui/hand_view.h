#ifndef VIEW_PREFAB_UI_HAND_VIEW_H
#define VIEW_PREFAB_UI_HAND_VIEW_H

#include <vector>

#include "raylib.h"

#include "data/render/hud_params.h"
#include "data/unit/unit.h"
#include "view/prefab/ui/ui_context.h"

namespace view
{
class TextureRegistry;
}

namespace view::ui
{
class HandView
{
public:
    void Update(UiInput& input, float dt);
    void Draw(UiContext& ui, const TextureRegistry& textures, RenderTexture2D& target);

    bool Dragging() const { return dragging_ >= 0; }
    data::UnitType DraggedType() const;
    Vector2 DragScreenPos() const { return dragPos_; }
    data::HandParams& ParamsRef() { return params_; }

    bool HasHighlight() const { return dragging_ >= 0 || hovered_ >= 0; }
    data::UnitType HighlightType() const;

    bool TakeDrop(int& slot, data::UnitType& type, Vector2& screenPos);
    void MarkPlayed(int slot);

private:
    struct Slot
    {
        data::UnitType type;
        int chargesLeft = 1;
        float hover = 0.0f;
        float drag = 0.0f;
    };

    void EnsureInit();
    float FanAngle(int i) const;
    Vector2 FanCenter(int i) const;
    void Transform(int i, Vector2& center, float& angle, float& scale) const;
    void DrawSlot(UiContext& ui, const TextureRegistry& textures, RenderTexture2D& target, int i);

    std::vector<Slot> slots_;
    data::HandParams params_;
    int hovered_ = -1;
    int dragging_ = -1;
    Vector2 dragPos_{};
    bool init_ = false;

    bool hasDrop_ = false;
    int dropSlot_ = -1;
    data::UnitType dropType_ = data::UnitType::Infantry;
    Vector2 dropPos_{};
};
}

#endif
