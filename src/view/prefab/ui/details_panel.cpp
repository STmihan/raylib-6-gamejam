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

    struct StatRow
    {
        const char* icon;
        int base;
        int merged;
    };

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

    const float pad = 16.0f;
    const float closeSz = 22.0f;

    Rectangle close = {rect.x + rect.width - pad - closeSz, rect.y + pad * 0.7f, closeSz, closeSz};
    atlas.DrawSprite("close-button", close);

    int cost = def.cost;
    if (donor >= 0) cost += data::CardDefOf(static_cast<data::UnitType>(donor)).cost;
    Rectangle badge = {rect.x + pad * 0.4f, rect.y - 8.0f, 42.0f, 30.0f};
    atlas.DrawSprite("res-fill", badge);
    char costText[8];
    std::snprintf(costText, sizeof(costText), "%d", cost);
    LabelCentered(ui, costText, badge, 18.0f, DarkInk, true);

    Rectangle nameBox = {rect.x + pad + 34.0f, rect.y + pad, rect.width - pad * 2.0f - closeSz - 42.0f, 38.0f};
    atlas.DrawNPatch("panel-base", nameBox);
    LabelCentered(ui, def.name, nameBox, 20.0f, Ink, true);

    float bodyTop = nameBox.y + nameBox.height + 16.0f;

    StatRow rows[5] = {
        {"stat-attack", base.baseDamage, merged.baseDamage},
        {"stat-hp", base.hp, merged.hp},
        {"stat-shield", base.armorHits, merged.armorHits},
        {"stat-ms", static_cast<int>(base.moveSpeed + 0.5f), static_cast<int>(merged.moveSpeed + 0.5f)},
        {"stat-range", base.attackRange, merged.attackRange},
    };

    const float iconSz = 26.0f;
    const float rowH = 34.0f;
    float statX = rect.x + pad + 4.0f;
    for (int i = 0; i < 5; i++)
    {
        float ry = bodyTop + static_cast<float>(i) * rowH;
        bool changed = donor >= 0 && rows[i].merged != rows[i].base;
        Color valueColor = changed ? Purple : Ink;
        atlas.DrawSprite(rows[i].icon, Rectangle{statX, ry, iconSz, iconSz});
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%d", rows[i].merged);
        Label(ui, buf, Vector2{statX + iconSz + 10.0f, ry + 3.0f}, 20.0f, valueColor, true);
    }

    float previewH = 5.0f * rowH;
    Rectangle previewBox = {rect.x + rect.width * 0.40f, bodyTop - 8.0f, rect.width * 0.52f, previewH};
    Icon(ui, textures.Preview(type), previewBox, WHITE);

    float descTop = bodyTop + previewH + 16.0f;
    Rectangle descBox = {rect.x + pad, descTop, rect.width - pad * 2.0f,
                         rect.y + rect.height - pad - descTop};
    atlas.DrawNPatch("panel-base", descBox);

    const float descPad = 12.0f;
    const float descSize = 15.0f;
    const float indent = 14.0f;
    const float bulletD = 5.0f;
    float lineW = descBox.width - descPad * 2.0f - indent;
    float y = descBox.y + descPad;

    const std::vector<std::string>& points = data::Rules().cardDescription[static_cast<std::size_t>(type)];
    for (const std::string& point : points)
    {
        Bullet(textures, descBox.x + descPad, y + descSize * 0.45f, bulletD, DimInk);
        Rectangle line = {descBox.x + descPad + indent, y, lineW, descSize};
        float h = DrawRichText(ui, point, line, descSize, DimInk);
        y += h + 4.0f;
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
