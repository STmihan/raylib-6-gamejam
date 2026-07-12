#include "view/prefab/ui/game_over.h"

#include <cstdio>

#include "data/economy/economy.h"
#include "data/time/time_config.h"
#include "data/unit/unit_types.h"
#include "logic/state/game_state.h"
#include "view/prefab/ui/ui_widgets.h"

namespace view::ui
{
namespace
{
    const Color Ink = {236, 232, 216, 255};
    const Color Dim = {198, 193, 178, 255};
    const Color WinColor = {120, 214, 140, 255};
    const Color LoseColor = {230, 96, 84, 255};
}

void DrawGameOver(UiContext& ui, const logic::GameState& state)
{
    float sw = static_cast<float>(GetScreenWidth());
    float sh = static_cast<float>(GetScreenHeight());
    ui.Theme().Fill(Rectangle{0.0f, 0.0f, sw, sh}, Color{0, 0, 0, 178});

    const float pw = 380.0f;
    const float ph = 292.0f;
    Rectangle panel = {(sw - pw) * 0.5f, (sh - ph) * 0.5f, pw, ph};
    if (ui.Atlas().Ready()) ui.Atlas().DrawNPatch("panel-base", panel);
    else ui.Theme().Panel(panel, Color{28, 32, 24, 245});

    bool playerWon = state.winner == data::TeamIndex(data::PlayerTeam);
    const float pad = 28.0f;
    float y = panel.y + pad;

    Rectangle titleRect = {panel.x, y, panel.width, 52.0f};
    LabelCentered(ui, playerWon ? "VICTORY" : "DEFEAT", titleRect, 46.0f, playerWon ? WinColor : LoseColor, true);
    y += 58.0f;

    Rectangle subRect = {panel.x, y, panel.width, 24.0f};
    LabelCentered(ui, "Press R to restart", subRect, 18.0f, Dim, false);
    y += 46.0f;

    int baseHp[2] = {0, 0};
    for (int i = 0; i < data::MaxEntities; i++)
    {
        const logic::Entity& e = state.entities[i];
        if (e.kind != logic::EntityKind::Base) continue;
        baseHp[data::TeamIndex(e.team)] = e.hp > 0 ? e.hp : 0;
    }
    int top = data::TeamIndex(data::Team::Top);
    int bottom = data::TeamIndex(data::Team::Bottom);

    int elapsed = static_cast<int>(state.tick / data::TickRate);
    bool overtime = elapsed >= data::MatchDurationSeconds();
    int shownTime = overtime ? elapsed - data::MatchDurationSeconds() : data::MatchDurationSeconds() - elapsed;
    if (shownTime < 0) shownTime = 0;
    char timeStr[16];
    std::snprintf(timeStr, sizeof(timeStr), "%s%d:%02d", overtime ? "-" : "", shownTime / 60, shownTime % 60);

    const Color Blue = {80, 150, 235, 255};
    const Color Red = {235, 85, 70, 255};
    const Color Grid = {236, 232, 216, 45};

    float contentX = panel.x + pad;
    float contentW = panel.width - pad * 2.0f;
    float labelColW = 60.0f;
    float dataColW = (contentW - labelColW) / 3.0f;
    float col1 = contentX + labelColW + dataColW * 0.5f;
    float col2 = col1 + dataColW;
    float col3 = col2 + dataColW;

    float headerH = 36.0f;
    float rowH = 44.0f;
    float tableTop = y + 6.0f;
    float dataTop = tableTop + headerH;

    ui.Theme().Fill(Rectangle{contentX, dataTop, contentW, 2.0f}, Grid);
    ui.Theme().Fill(Rectangle{contentX + labelColW, tableTop, 2.0f, headerH + rowH * 2.0f}, Grid);

    auto icon = [&](const char* name, float cx, float cy, float sz) {
        Rectangle src = ui.Atlas().Source(name);
        float w = sz;
        float h = sz;
        if (src.width > 0.0f && src.height > 0.0f)
        {
            if (src.width >= src.height) h = sz * src.height / src.width;
            else w = sz * src.width / src.height;
        }
        ui.Atlas().DrawSprite(name, Rectangle{cx - w * 0.5f, cy - h * 0.5f, w, h});
    };
    auto square = [&](float cx, float cy, float sz, Color color) {
        ui.Theme().Fill(Rectangle{cx - sz * 0.5f, cy - sz * 0.5f, sz, sz}, color);
    };
    auto cell = [&](int value, float cx, float rowTop, float rowHeight) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%d", value);
        LabelCentered(ui, buf, Rectangle{cx - dataColW * 0.5f, rowTop, dataColW, rowHeight}, 20.0f, Ink, true);
    };

    float headerCy = tableTop + headerH * 0.5f;
    icon("timer-icon", col1, headerCy, 26.0f);
    icon("stat-hp", col2, headerCy, 26.0f);
    icon("cards-icon", col3, headerCy, 26.0f);

    float row1 = dataTop;
    float row2 = dataTop + rowH;
    float squareX = contentX + labelColW * 0.5f;
    square(squareX, row1 + rowH * 0.5f, 26.0f, Blue);
    square(squareX, row2 + rowH * 0.5f, 26.0f, Red);

    LabelCentered(ui, timeStr, Rectangle{col1 - dataColW * 0.5f, row1, dataColW, rowH * 2.0f}, 20.0f, Ink, true);

    cell(baseHp[bottom], col2, row1, rowH);
    cell(baseHp[top], col2, row2, rowH);
    cell(state.unitsDeployed[bottom], col3, row1, rowH);
    cell(state.unitsDeployed[top], col3, row2, rowH);
}
}
