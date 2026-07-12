#include "view/prefab/ui/details_panel.h"

#include <cstdio>
#include <string>
#include <vector>

#include "data/balance/balance.h"
#include "data/card/card.h"
#include "data/unit/unit.h"
#include "view/prefab/registries/texture_registry.h"
#include "view/prefab/ui/ui_widgets.h"

namespace view::ui
{
namespace
{
    const Color Ink = {236, 232, 216, 255};
    const Color DimInk = {206, 201, 184, 255};
    const Color Purple = {176, 132, 240, 255};
    const Color DarkInk = {40, 32, 8, 255};

    void Bullet(const TextureRegistry& textures, float x, float y, float diameter, Color color)
    {
        Rectangle src = {0.0f, 0.0f, 1.0f, 1.0f};
        DrawTexturePro(textures.White(), src, Rectangle{x, y, diameter, diameter}, Vector2{0.0f, 0.0f}, 0.0f,
                       color);
    }
}

void DrawDetailsPanel(UiContext& ui, const TextureRegistry& textures, Rectangle rect, data::UnitType type,
                      int donor)
{
    UiAtlas& atlas = ui.Atlas();
    if (!atlas.Ready()) return;

    atlas.DrawNPatch("panel-base", rect);
    ui.Input().Block(rect);

    data::CardDef def = data::CardDefOf(type);
    data::UnitStats base = data::UnitStatsOf(type);
    data::UnitStats merged = data::MergedStats(type, donor);

    const float s = rect.width / 300.0f;
    const float pad = 16.0f * s;
    const float closeSz = 22.0f * s;

    int cost = def.cost;
    if (donor >= 0) cost += data::CardDefOf(static_cast<data::UnitType>(donor)).cost;
    Rectangle badge = {rect.x + pad * 0.4f, rect.y - 8.0f * s, 42.0f * s, 30.0f * s};
    atlas.DrawSprite("res-fill", badge);
    char costText[8];
    std::snprintf(costText, sizeof(costText), "%d", cost);
    LabelCentered(ui, costText, badge, 18.0f * s, DarkInk, true);

    Rectangle nameBox = {rect.x + pad + 34.0f * s, rect.y + pad, rect.width - pad * 2.0f - 34.0f * s, 50.0f * s};
    atlas.DrawNPatch("panel-base", nameBox);
    LabelCentered(ui, def.name, nameBox, 28.0f * s, Ink, true);

    Rectangle close = {rect.x + rect.width - pad - closeSz, rect.y + pad * 0.7f, closeSz, closeSz};
    atlas.DrawSprite("close-button", close);

    float bodyTop = nameBox.y + nameBox.height + 16.0f * s;

    const float iconSz = 26.0f * s;
    const float rowH = 34.0f * s;
    const float statFont = 20.0f * s;
    float statX = rect.x + pad + 4.0f * s;

    auto colorFor = [&](int b, int m) { return (donor >= 0 && m != b) ? Purple : Ink; };
    auto drawStat = [&](float x, float ry, const char* icon, int value, float isz, Color color,
                        bool keepAspect) -> float {
        float iw = isz;
        if (keepAspect)
        {
            Rectangle src = atlas.Source(icon);
            if (src.height > 0.0f) iw = isz * (src.width / src.height);
        }
        atlas.DrawSprite(icon, Rectangle{x, ry + (iconSz - isz) * 0.5f, iw, isz});
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%d", value);
        float tx = x + iw + 8.0f * s;
        Label(ui, buf, Vector2{tx, ry + 3.0f * s}, statFont, color, true);
        return tx + ui.Text().Width(buf, statFont);
    };

    drawStat(statX, bodyTop, "stat-attack", merged.baseDamage, iconSz,
             colorFor(base.baseDamage, merged.baseDamage), false);

    float hpRight = drawStat(statX, bodyTop + rowH, "stat-hp", merged.hp, iconSz,
                             colorFor(base.hp, merged.hp), false);
    if (merged.armorHits > 0)
        drawStat(hpRight + 12.0f * s, bodyTop + rowH, "stat-shield", merged.armorHits, iconSz,
                 colorFor(base.armorHits, merged.armorHits), false);

    drawStat(statX, bodyTop + rowH * 2.0f, "stat-ms", static_cast<int>(merged.moveSpeed + 0.5f), iconSz,
             colorFor(static_cast<int>(base.moveSpeed + 0.5f), static_cast<int>(merged.moveSpeed + 0.5f)), false);

    drawStat(statX, bodyTop + rowH * 3.0f, "stat-range", merged.attackRange, iconSz,
             colorFor(base.attackRange, merged.attackRange), false);

    drawStat(statX, bodyTop + rowH * 4.0f, "charge-fill", def.charges, iconSz * 0.62f, Ink, true);

    float previewH = 5.0f * rowH;
    Rectangle previewBox = {rect.x + rect.width * 0.40f, bodyTop - 8.0f * s, rect.width * 0.52f, previewH};
    Icon(ui, textures.Preview(type), previewBox, WHITE);

    float descTop = bodyTop + previewH + 16.0f * s;
    Rectangle descBox = {rect.x + pad, descTop, rect.width - pad * 2.0f,
                         rect.y + rect.height - pad - descTop};
    atlas.DrawNPatch("panel-base", descBox);

    const float descPad = 12.0f * s;
    const float descSize = 20.0f * s;
    const float indent = 14.0f * s;
    const float bulletD = 5.0f * s;
    float lineW = descBox.width - descPad * 2.0f - indent;
    float y = descBox.y + descPad;

    const std::vector<std::string>& points = data::Rules().cardDescription[static_cast<std::size_t>(type)];
    for (const std::string& point : points)
    {
        Bullet(textures, descBox.x + descPad, y + descSize * 0.45f, bulletD, DimInk);
        Rectangle line = {descBox.x + descPad + indent, y, lineW, descSize};
        float h = DrawRichText(ui, point, line, descSize, DimInk, WHITE, true);
        y += h + 4.0f * s;
    }

    if (donor >= 0)
    {
        const std::string& grant = data::Rules().cardMergeGrant[static_cast<std::size_t>(donor)];
        if (!grant.empty())
        {
            Bullet(textures, descBox.x + descPad, y + descSize * 0.45f, bulletD, Purple);
            Rectangle line = {descBox.x + descPad + indent, y, lineW, descSize};
            DrawRichText(ui, grant, line, descSize, Purple, Purple, true);
        }
    }
}
}
