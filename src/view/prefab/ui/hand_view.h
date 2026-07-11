#ifndef VIEW_PREFAB_UI_HAND_VIEW_H
#define VIEW_PREFAB_UI_HAND_VIEW_H

#include <vector>

#include "raylib.h"

#include "data/render/hud_params.h"
#include "data/unit/unit.h"
#include "logic/state/game_state.h"
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
    void Update(UiInput& input, float dt, const logic::GameState& state);
    void Draw(UiContext& ui, const TextureRegistry& textures, RenderTexture2D& target);
    void DrawDragZone(const TextureRegistry& textures) const;

    bool Dragging() const { return dragging_ >= 0; }
    data::UnitType DraggedType() const;
    Vector2 DragScreenPos() const { return dragPos_; }
    data::HandParams& ParamsRef() { return params_; }

    bool HasHighlight() const { return dragging_ >= 0 || hovered_ >= 0; }
    data::UnitType HighlightType() const;
    int HighlightCost() const;
    bool DraggedAirdrop() const;
    bool DragDeploys() const;

    bool TakeDrop(int& slot, Vector2& screenPos);
    bool TakeMerge(int& host, int& donor);

private:
    struct Slot
    {
        data::UnitType type = data::UnitType::Infantry;
        int donor = -1;
        int chargesLeft = 1;
        float hover = 0.0f;
        float drag = 0.0f;
    };

    void Sync(const logic::GameState& state);
    float FanAngle(int i) const;
    Vector2 FanCenter(int i) const;
    void Transform(int i, Vector2& center, float& angle, float& scale) const;
    void DrawSlot(UiContext& ui, const TextureRegistry& textures, RenderTexture2D& target, int i);
    void DrawMergeBadge(UiContext& ui, const TextureRegistry& textures, Vector2 pos, data::UnitType donorType) const;
    int HostUnderCursor(Vector2 m, int exclude) const;
    bool CanMerge(int host, int donorIdx) const;
    int ValidMergeHost(Vector2 m) const;
    bool DragOutside() const;
    int SlotCost(const Slot& s) const;

    std::vector<Slot> slots_;
    data::HandParams params_;
    int hovered_ = -1;
    int dragging_ = -1;
    int mergeHost_ = -1;
    Vector2 dragPos_{};

    bool hasDrop_ = false;
    int dropSlot_ = -1;
    Vector2 dropPos_{};

    bool hasMerge_ = false;
    int mergeHostOut_ = -1;
    int mergeDonorOut_ = -1;
};
}

#endif
