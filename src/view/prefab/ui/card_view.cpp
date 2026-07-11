#include "view/prefab/ui/card_view.h"

#include <cstdio>

#include "view/prefab/ui/ui_widgets.h"

namespace view::ui
{
namespace
{
    const Color Frame = {28, 32, 24, 255};
    const Color Window = {17, 20, 13, 255};
    const Color Strip = {44, 50, 36, 255};
    const Color ChargeSlot = {14, 17, 11, 255};
    const Color Gold = {240, 192, 58, 255};
    const Color DarkInk = {40, 32, 8, 255};
    const Color LightInk = {240, 236, 221, 255};

    data::CrystalStyle ChargeStyle()
    {
        data::CrystalStyle s;
        s.top = {1.0f, 0.85f, 0.42f};
        s.bottom = {0.86f, 0.52f, 0.08f};
        s.gloss = 0.4f;
        s.split = 0.56f;
        s.edge = 0.1f;
        s.outline = {0.20f, 0.12f, 0.0f};
        s.outlineWidth = 1.0f;
        return s;
    }
}

void DrawCard(UiContext& ui, Rectangle rect, const data::CardDef& def, const Texture2D& portrait,
              int chargesLeft, int cost, int donor, const Texture2D& donorIcon)
{
    ui.Theme().Panel(rect, Frame);

    const float pad = 8.0f;
    Rectangle window = {rect.x + pad, rect.y + pad, rect.width - 2.0f * pad, rect.height * 0.58f};
    ui.Theme().Chip(window, Window);
    Icon(ui, portrait, window, WHITE);

    Rectangle name = {rect.x + pad, window.y + window.height + 4.0f, rect.width - 2.0f * pad, 24.0f};
    ui.Theme().Chip(name, Strip);
    LabelCentered(ui, def.name, name, 15.0f, LightInk, true);

    int remaining = chargesLeft;
    int n = remaining > def.charges ? remaining : def.charges;
    if (n > 1)
    {
        const float chargeH = 7.0f;
        const float gap = 3.0f;
        float barW = rect.width * 0.46f;
        Rectangle bar = {rect.x + (rect.width - barW) * 0.5f, rect.y + rect.height - pad - chargeH, barW,
                         chargeH};
        float cw = (bar.width - gap * static_cast<float>(n - 1)) / static_cast<float>(n);
        static const data::CrystalStyle chargeStyle = ChargeStyle();
        for (int i = 0; i < n; i++)
        {
            Rectangle seg = {bar.x + static_cast<float>(i) * (cw + gap), bar.y, cw, bar.height};
            if (i < remaining) ui.Theme().Crystal(seg, chargeStyle);
            else ui.Theme().Chip(seg, ChargeSlot);
        }
    }

    Rectangle badge = {rect.x - 6.0f, rect.y - 6.0f, 30.0f, 30.0f};
    ui.Theme().Chip(badge, Gold);
    char costText[8];
    std::snprintf(costText, sizeof(costText), "%d", cost);
    LabelCentered(ui, costText, badge, 18.0f, DarkInk, true);

    if (donor >= 0)
    {
        const Color DonorChip = {126, 92, 208, 255};
        Rectangle dbadge = {rect.x + rect.width - 24.0f, rect.y - 6.0f, 30.0f, 30.0f};
        ui.Theme().Chip(dbadge, DonorChip);
        Rectangle inner = {dbadge.x + 4.0f, dbadge.y + 4.0f, dbadge.width - 8.0f, dbadge.height - 8.0f};
        Icon(ui, donorIcon, inner, LightInk);
    }
}

void DrawCardTransformed(UiContext& ui, RenderTexture2D& target, const data::CardDef& def,
                         const Texture2D& portrait, Vector2 center, float rotationDeg, float scale,
                         float cardW, float cardH, int chargesLeft, int cost, int donor,
                         const Texture2D& donorIcon)
{
    float tw = static_cast<float>(target.texture.width);
    float th = static_cast<float>(target.texture.height);
    float mx = (tw - cardW) * 0.5f;
    float my = (th - cardH) * 0.5f;

    BeginTextureMode(target);
    ClearBackground(BLANK);
    DrawCard(ui, Rectangle{mx, my, cardW, cardH}, def, portrait, chargesLeft, cost, donor, donorIcon);
    EndTextureMode();

    Rectangle src = {0.0f, 0.0f, tw, -th};
    Rectangle dst = {center.x, center.y, tw * scale, th * scale};
    DrawTexturePro(target.texture, src, dst, Vector2{tw * scale * 0.5f, th * scale * 0.5f}, rotationDeg, WHITE);
}
}
