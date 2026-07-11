#include "view/prefab/ui/hand_view.h"

#include <cmath>
#include <cstddef>

#include "data/card/card.h"
#include "view/prefab/registries/texture_registry.h"
#include "view/prefab/ui/card_view.h"
#include "view/prefab/ui/ui_widgets.h"

namespace view::ui
{
namespace
{
    constexpr float CardW = 118.0f;
    constexpr float CardH = 166.0f;
    constexpr float DegToRad = 0.0174532925f;

    bool HitCard(Vector2 m, Vector2 center, float angleDeg, float w, float h)
    {
        float a = -angleDeg * DegToRad;
        float dx = m.x - center.x;
        float dy = m.y - center.y;
        float lx = dx * std::cos(a) - dy * std::sin(a);
        float ly = dx * std::sin(a) + dy * std::cos(a);
        return std::fabs(lx) <= w * 0.5f && std::fabs(ly) <= h * 0.5f;
    }
}

void HandView::Sync(const logic::GameState& state)
{
    const logic::PlayerCards& player =
        state.players[static_cast<std::size_t>(data::TeamIndex(data::PlayerTeam))];
    if (static_cast<int>(slots_.size()) != data::HandSize)
    {
        slots_.assign(static_cast<std::size_t>(data::HandSize), Slot{});
    }
    for (int i = 0; i < data::HandSize; i++)
    {
        Slot& s = slots_[static_cast<std::size_t>(i)];
        const logic::HandSlot& src = player.hand[static_cast<std::size_t>(i)];
        s.type = src.type;
        s.donor = src.donor;
        s.chargesLeft = src.chargesLeft;
    }
}

float HandView::FanAngle(int i) const
{
    int count = static_cast<int>(slots_.size());
    return (static_cast<float>(i) - static_cast<float>(count - 1) * 0.5f) * params_.step;
}

Vector2 HandView::FanCenter(int i) const
{
    float a = FanAngle(i) * DegToRad;
    return Vector2{params_.pivot.x + params_.radius * std::sin(a),
                   params_.pivot.y - params_.radius * std::cos(a)};
}

void HandView::Transform(int i, Vector2& center, float& angle, float& scale) const
{
    Vector2 fc = FanCenter(i);
    float fa = FanAngle(i);
    const Slot& s = slots_[static_cast<std::size_t>(i)];

    if (dragging_ == i)
    {
        center = dragPos_;
        angle = 0.0f;
        scale = 1.0f - s.drag;
    }
    else
    {
        angle = fa * (1.0f - s.hover);
        center = Vector2{fc.x, fc.y - params_.raise * s.hover};
        scale = 1.0f + params_.hoverScale * s.hover;
    }
}

void HandView::Update(UiInput& input, float dt, const logic::GameState& state)
{
    Sync(state);
    Vector2 m = input.Mouse();
    int count = static_cast<int>(slots_.size());

    if (dragging_ < 0)
    {
        hovered_ = -1;
        for (int i = count - 1; i >= 0; i--)
        {
            Vector2 c;
            float ang;
            float sc;
            Transform(i, c, ang, sc);
            if (HitCard(m, c, ang, CardW * sc, CardH * sc))
            {
                hovered_ = i;
                break;
            }
        }
        if (input.Pressed() && hovered_ >= 0)
        {
            dragging_ = hovered_;
            hovered_ = -1;
        }
    }

    if (dragging_ >= 0)
    {
        dragPos_ = m;
        mergeHost_ = ValidMergeHost(m);
        if (!input.Down())
        {
            if (mergeHost_ >= 0)
            {
                hasMerge_ = true;
                mergeHostOut_ = mergeHost_;
                mergeDonorOut_ = dragging_;
            }
            else if (DragOutside())
            {
                hasDrop_ = true;
                dropSlot_ = dragging_;
                dropPos_ = m;
            }
            dragging_ = -1;
            mergeHost_ = -1;
        }
    }

    float k = dt / params_.hoverTime;
    if (k > 1.0f) k = 1.0f;
    for (int i = 0; i < count; i++)
    {
        Slot& s = slots_[static_cast<std::size_t>(i)];
        float hoverTarget = (i == hovered_) ? 1.0f : 0.0f;
        s.hover += (hoverTarget - s.hover) * k;

        float dragTarget = 0.0f;
        if (dragging_ == i)
        {
            Vector2 slotCenter = FanCenter(i);
            float dx = dragPos_.x - slotCenter.x;
            float dy = dragPos_.y - slotCenter.y;
            dragTarget = std::sqrt(dx * dx + dy * dy) > params_.dragRadius ? 1.0f : 0.0f;
        }
        s.drag += (dragTarget - s.drag) * k;
    }

    for (int i = 0; i < count; i++)
    {
        Vector2 c;
        float ang;
        float sc;
        Transform(i, c, ang, sc);
        input.Block(Rectangle{c.x - CardW * sc * 0.5f, c.y - CardH * sc * 0.5f, CardW * sc, CardH * sc});
    }
}

void HandView::DrawSlot(UiContext& ui, const TextureRegistry& textures, RenderTexture2D& target, int i)
{
    Vector2 c;
    float ang;
    float sc;
    Transform(i, c, ang, sc);
    if (sc <= 0.01f) return;
    const Slot& s = slots_[static_cast<std::size_t>(i)];
    const Texture2D& donorIcon =
        s.donor >= 0 ? textures.MergeIcon(static_cast<data::UnitType>(s.donor)) : textures.White();
    DrawCardTransformed(ui, target, data::CardDefOf(s.type), textures.Preview(s.type), c, ang, sc, CardW, CardH,
                        s.chargesLeft, SlotCost(s), s.donor, donorIcon);
}

void HandView::DrawMergeBadge(UiContext& ui, const TextureRegistry& textures, Vector2 pos,
                              data::UnitType donorType) const
{
    Rectangle chip = {pos.x - 26.0f, pos.y - 26.0f, 52.0f, 52.0f};
    ui.Theme().Chip(chip, Color{126, 92, 208, 170});
    Rectangle inner = {chip.x + 8.0f, chip.y + 8.0f, chip.width - 16.0f, chip.height - 16.0f};
    Icon(ui, textures.MergeIcon(donorType), inner, Color{255, 255, 255, 220});
}

int HandView::ValidMergeHost(Vector2 m) const
{
    int host = HostUnderCursor(m, dragging_);
    return CanMerge(host, dragging_) ? host : -1;
}

bool HandView::CanMerge(int host, int donorIdx) const
{
    if (host < 0 || donorIdx < 0 || host == donorIdx) return false;
    const Slot& h = slots_[static_cast<std::size_t>(host)];
    const Slot& d = slots_[static_cast<std::size_t>(donorIdx)];
    return d.donor < 0 && h.donor < 0 && h.type != d.type;
}

void HandView::DrawDragZone(const TextureRegistry& textures) const
{
    if (dragging_ < 0) return;
    Vector2 c = FanCenter(dragging_);
    float r = params_.dragRadius;
    const int seg = 48;
    Rectangle source = {0.0f, 0.0f, 1.0f, 1.0f};
    for (int i = 0; i < seg; i++)
    {
        float a = 6.2831853f * static_cast<float>(i) / static_cast<float>(seg);
        float px = c.x + std::cos(a) * r;
        float py = c.y + std::sin(a) * r;
        DrawTexturePro(textures.White(), source, Rectangle{px - 2.0f, py - 2.0f, 4.0f, 4.0f}, Vector2{0.0f, 0.0f},
                       0.0f, Color{120, 220, 255, 220});
    }
}

bool HandView::DragOutside() const
{
    if (dragging_ < 0) return false;
    Vector2 slotCenter = FanCenter(dragging_);
    float dx = dragPos_.x - slotCenter.x;
    float dy = dragPos_.y - slotCenter.y;
    return std::sqrt(dx * dx + dy * dy) > params_.dragRadius;
}

bool HandView::DragDeploys() const
{
    return dragging_ >= 0 && mergeHost_ < 0 && DragOutside();
}

int HandView::HostUnderCursor(Vector2 m, int exclude) const
{
    int count = static_cast<int>(slots_.size());
    for (int i = 0; i < count; i++)
    {
        if (i == exclude) continue;
        Vector2 c;
        float ang;
        float sc;
        Transform(i, c, ang, sc);
        if (HitCard(m, c, ang, CardW * sc, CardH * sc)) return i;
    }
    return -1;
}

int HandView::SlotCost(const Slot& s) const
{
    int cost = data::CardDefOf(s.type).cost;
    if (s.donor >= 0) cost += data::CardDefOf(static_cast<data::UnitType>(s.donor)).cost;
    return cost;
}

void HandView::Draw(UiContext& ui, const TextureRegistry& textures, RenderTexture2D& target)
{
    int count = static_cast<int>(slots_.size());
    for (int i = 0; i < count; i++)
    {
        if (i == hovered_ || i == dragging_) continue;
        DrawSlot(ui, textures, target, i);
    }
    if (hovered_ >= 0 && hovered_ != dragging_) DrawSlot(ui, textures, target, hovered_);
    if (dragging_ >= 0)
    {
        if (mergeHost_ >= 0)
            DrawMergeBadge(ui, textures, dragPos_, slots_[static_cast<std::size_t>(dragging_)].type);
        else
            DrawSlot(ui, textures, target, dragging_);
    }
}

data::UnitType HandView::DraggedType() const
{
    if (dragging_ < 0) return data::UnitType::Infantry;
    return slots_[static_cast<std::size_t>(dragging_)].type;
}

data::UnitType HandView::HighlightType() const
{
    if (dragging_ >= 0) return slots_[static_cast<std::size_t>(dragging_)].type;
    if (hovered_ >= 0) return slots_[static_cast<std::size_t>(hovered_)].type;
    return data::UnitType::Infantry;
}

int HandView::HighlightCost() const
{
    if (dragging_ >= 0) return SlotCost(slots_[static_cast<std::size_t>(dragging_)]);
    if (hovered_ >= 0) return SlotCost(slots_[static_cast<std::size_t>(hovered_)]);
    return 0;
}

bool HandView::DraggedAirdrop() const
{
    if (dragging_ < 0) return false;
    const Slot& s = slots_[static_cast<std::size_t>(dragging_)];
    return s.type == data::UnitType::Plane || s.donor == static_cast<int>(data::UnitType::Plane);
}

bool HandView::TakeDrop(int& slot, Vector2& screenPos)
{
    if (!hasDrop_) return false;
    hasDrop_ = false;
    slot = dropSlot_;
    screenPos = dropPos_;
    return true;
}

bool HandView::TakeMerge(int& host, int& donor)
{
    if (!hasMerge_) return false;
    hasMerge_ = false;
    host = mergeHostOut_;
    donor = mergeDonorOut_;
    return true;
}
}
