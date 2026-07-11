#include "view/prefab/ui/hand_view.h"

#include <cmath>

#include "data/card/card.h"
#include "view/prefab/registries/texture_registry.h"
#include "view/prefab/ui/card_view.h"

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

void HandView::EnsureInit()
{
    if (init_) return;
    data::UnitType types[3] = {data::UnitType::Infantry, data::UnitType::Tank, data::UnitType::Plane};
    for (data::UnitType t : types)
    {
        Slot s;
        s.type = t;
        s.chargesLeft = data::CardDefOf(t).charges;
        slots_.push_back(s);
    }
    init_ = true;
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

void HandView::Update(UiInput& input, float dt)
{
    EnsureInit();
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
        if (!input.Down())
        {
            hasDrop_ = true;
            dropSlot_ = dragging_;
            dropType_ = slots_[static_cast<std::size_t>(dragging_)].type;
            dropPos_ = m;
            dragging_ = -1;
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
            float dx = dragPos_.x - params_.anchor.x;
            float dy = dragPos_.y - params_.anchor.y;
            dragTarget = std::sqrt(dx * dx + dy * dy) / params_.dragRadius;
            if (dragTarget > 1.0f) dragTarget = 1.0f;
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
    DrawCardTransformed(ui, target, data::CardDefOf(s.type), textures.Preview(s.type), c, ang, sc, CardW, CardH,
                        s.chargesLeft);
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
    if (dragging_ >= 0) DrawSlot(ui, textures, target, dragging_);
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

bool HandView::TakeDrop(int& slot, data::UnitType& type, Vector2& screenPos)
{
    if (!hasDrop_) return false;
    hasDrop_ = false;
    slot = dropSlot_;
    type = dropType_;
    screenPos = dropPos_;
    return true;
}

void HandView::MarkPlayed(int slot)
{
    if (slot < 0 || slot >= static_cast<int>(slots_.size())) return;
    Slot& s = slots_[static_cast<std::size_t>(slot)];
    s.chargesLeft--;
    if (s.chargesLeft <= 0)
    {
        s.type = static_cast<data::UnitType>(GetRandomValue(0, data::UnitTypeCount - 1));
        s.chargesLeft = data::CardDefOf(s.type).charges;
        s.hover = 0.0f;
        s.drag = 0.0f;
    }
}
}
