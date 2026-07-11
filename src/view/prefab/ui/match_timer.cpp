#include "view/prefab/ui/match_timer.h"

#include <cstdio>

#include "data/economy/economy.h"
#include "data/time/time_config.h"
#include "view/prefab/ui/ui_context.h"
#include "view/prefab/ui/ui_widgets.h"

namespace view::ui
{
void DrawMatchTimer(UiContext& ui, Rectangle rect, std::uint64_t tick)
{
    int elapsed = static_cast<int>(tick / data::TickRate);
    bool overtime = elapsed >= data::MatchDurationSeconds;

    int shown = overtime ? elapsed - data::MatchDurationSeconds : data::MatchDurationSeconds - elapsed;
    if (shown < 0) shown = 0;
    char text[8];
    std::snprintf(text, sizeof(text), "%d:%02d", shown / 60, shown % 60);

    if (!overtime)
    {
        Panel(ui, rect, Color{28, 32, 24, 235});
        LabelCentered(ui, text, rect, 30.0f, RAYWHITE, true);
        return;
    }

    const Color white = {245, 245, 245, 255};
    const Color red = {176, 36, 32, 245};
    Rectangle outline = {rect.x - 3.0f, rect.y - 3.0f, rect.width + 6.0f, rect.height + 6.0f};
    ui.Theme().Panel(outline, white);
    Panel(ui, rect, red);
    LabelCentered(ui, text, rect, 30.0f, white, true);

    const char* stamp = "OVERTIME";
    float stampSize = 12.0f;
    Vector2 sz = ui.Text().Measure(stamp, stampSize);
    Vector2 origin = {sz.x * 0.5f, sz.y * 0.5f};
    Vector2 pos = {rect.x + rect.width * 0.5f, rect.y + rect.height - 9.0f};
    ui.Text().DrawRotated(stamp, pos, origin, -18.0f, stampSize, white, 0.0f, true);
}
}
