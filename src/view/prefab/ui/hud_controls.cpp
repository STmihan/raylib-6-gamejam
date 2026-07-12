#include "view/prefab/ui/hud_controls.h"

#include "audio/sound.h"
#include "data/balance/balance.h"
#include "view/prefab/ui/ui_widgets.h"

namespace view::ui
{
namespace
{
    constexpr float ButtonSize = 44.0f;
    constexpr float Gap = 8.0f;
    constexpr float OriginX = 146.0f;
    constexpr float OriginY = 12.0f;
    constexpr float PanelWidth = 184.0f;
    constexpr float Overlap = 42.0f;
    constexpr float OpenSpeed = 18.0f;

    constexpr float TrackInset = 40.0f;
    constexpr float TrackWidth = 128.0f;
    constexpr float TrackHeight = 16.0f;
    constexpr float TrackBorderWidth = 2.0f;

    constexpr float HelpPanelW = 500.0f;
    constexpr float HelpPanelH = 508.0f;
    constexpr float HelpOkW = 120.0f;
    constexpr float HelpOkH = 48.0f;

    const Color TrackBorder = {18, 22, 16, 255};
    const Color TrackBg = {52, 58, 44, 255};
    const Color TrackFill = {240, 192, 58, 255};
    const Color Ink = {236, 232, 216, 255};
    const Color Dim = {206, 201, 184, 255};

    void DrawCenteredSprite(UiAtlas& atlas, const char* name, Rectangle box, float scale, Color tint)
    {
        Rectangle src = atlas.Source(name);
        if (src.height <= 0.0f) return;
        float h = box.height * scale;
        float w = h * (src.width / src.height);
        Rectangle dst = {box.x + (box.width - w) * 0.5f, box.y + (box.height - h) * 0.5f, w, h};
        atlas.DrawSprite(name, dst, tint);
    }
}

HudControls::Layout HudControls::Compute() const
{
    Layout l;
    float step = ButtonSize + Gap;
    l.help = {OriginX, OriginY, ButtonSize, ButtonSize};
    l.pause = {OriginX + step, OriginY, ButtonSize, ButtonSize};
    l.sound = {OriginX + step * 2.0f, OriginY, ButtonSize, ButtonSize};
    float visLeft = l.sound.x + l.sound.width;
    l.panel = {visLeft - Overlap, OriginY, PanelWidth, ButtonSize};
    l.track = {l.panel.x + TrackInset, OriginY + (ButtonSize - TrackHeight) * 0.5f, TrackWidth, TrackHeight};
    return l;
}

Rectangle HudControls::HelpOkButton() const
{
    float sw = static_cast<float>(GetScreenWidth());
    float sh = static_cast<float>(GetScreenHeight());
    Rectangle panel = {(sw - HelpPanelW) * 0.5f, (sh - HelpPanelH) * 0.5f, HelpPanelW, HelpPanelH};
    return {panel.x + (panel.width - HelpOkW) * 0.5f, panel.y + panel.height - 28.0f - HelpOkH, HelpOkW, HelpOkH};
}

void HudControls::Update(UiContext& ui, float dt)
{
    UiInput& in = ui.Input();
    Layout l = Compute();

    if (helpOpen_)
    {
        if (in.Pressed(HelpOkButton()))
        {
            helpOpen_ = false;
            audio::Play("button-click");
        }
        in.Block(Rectangle{0.0f, 0.0f, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())});
        return;
    }

    if (in.Pressed(l.help))
    {
        helpOpen_ = true;
        audio::Play("button-click");
    }

    if (in.Pressed(l.pause) || IsKeyPressed(KEY_SPACE))
    {
        paused_ = !paused_;
        audio::Play("button-click");
    }

    if (in.Pressed(l.sound))
    {
        audio::Play("button-click");
        if (volume_ > 0.0f)
        {
            lastVolume_ = volume_;
            volume_ = 0.0f;
        }
        else
        {
            volume_ = lastVolume_ >= 0.1f ? lastVolume_ : 0.1f;
        }
    }

    bool overButton = in.Hover(l.sound);
    bool overPanel = panelOpen_ > 0.05f && in.Hover(l.panel);
    bool wantOpen = overButton || overPanel || draggingVolume_;
    float target = wantOpen ? 1.0f : 0.0f;
    float k = dt * OpenSpeed;
    if (k > 1.0f) k = 1.0f;
    panelOpen_ += (target - panelOpen_) * k;

    if (panelOpen_ > 0.5f && in.Pressed(l.track)) draggingVolume_ = true;
    if (!in.Down()) draggingVolume_ = false;
    if (draggingVolume_)
    {
        float t = (in.Mouse().x - l.track.x) / l.track.width;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        volume_ = t;
    }
    audio::SetMasterVolume(volume_);

    in.Block(l.help);
    in.Block(l.pause);
    in.Block(l.sound);
    if (panelOpen_ > 0.05f) in.Block(l.panel);
}

void HudControls::Draw(UiContext& ui)
{
    UiAtlas& atlas = ui.Atlas();
    if (!atlas.Ready()) return;
    UiInput& in = ui.Input();
    Layout l = Compute();

    bool helpHover = in.Hover(l.help);
    bool helpDown = helpHover && in.Down();
    const char* helpBg =
        helpDown ? "button-pressed" : ((helpOpen_ || helpHover) ? "button-hover" : "button-normal");
    atlas.DrawSprite(helpBg, l.help);
    DrawCenteredSprite(atlas, "question-icon", l.help, 0.5f, WHITE);

    bool pauseHover = in.Hover(l.pause);
    bool pauseDown = pauseHover && in.Down();
    const char* pauseBg =
        pauseDown ? "button-pressed" : ((paused_ || pauseHover) ? "button-hover" : "button-normal");
    atlas.DrawSprite(pauseBg, l.pause);
    DrawCenteredSprite(atlas, "pause-icon", l.pause, 0.36f, WHITE);

    if (panelOpen_ > 0.001f)
    {
        float slide = (1.0f - panelOpen_) * PanelWidth;
        float clipX = l.panel.x + Overlap;
        float clipW = PanelWidth - Overlap;
        BeginScissorMode(static_cast<int>(clipX), static_cast<int>(l.panel.y), static_cast<int>(clipW),
                         static_cast<int>(l.panel.height));

        Rectangle panel = {l.panel.x - slide, l.panel.y, l.panel.width, l.panel.height};
        atlas.DrawNPatch("panel-base", panel);

        Rectangle track = {l.track.x - slide, l.track.y, l.track.width, l.track.height};
        ui.Theme().Fill(track, TrackBorder);
        Rectangle bg = {track.x + TrackBorderWidth, track.y + TrackBorderWidth,
                        track.width - 2.0f * TrackBorderWidth, track.height - 2.0f * TrackBorderWidth};
        ui.Theme().Fill(bg, TrackBg);
        Rectangle fill = {bg.x, bg.y, bg.width * volume_, bg.height};
        ui.Theme().Fill(fill, TrackFill);
        EndScissorMode();
    }

    bool soundHover = in.Hover(l.sound);
    bool soundDown = soundHover && in.Down();
    const char* soundBg =
        soundDown ? "button-pressed" : ((panelOpen_ > 0.5f || soundHover) ? "button-hover" : "button-normal");
    atlas.DrawSprite(soundBg, l.sound);
    DrawCenteredSprite(atlas, "sound-icon", l.sound, 0.5f, WHITE);

    if (helpOpen_) DrawHelp(ui);
}

void HudControls::DrawHelp(UiContext& ui)
{
    UiAtlas& atlas = ui.Atlas();
    UiInput& in = ui.Input();

    float sw = static_cast<float>(GetScreenWidth());
    float sh = static_cast<float>(GetScreenHeight());
    ui.Theme().Fill(Rectangle{0.0f, 0.0f, sw, sh}, Color{0, 0, 0, 178});

    Rectangle panel = {(sw - HelpPanelW) * 0.5f, (sh - HelpPanelH) * 0.5f, HelpPanelW, HelpPanelH};
    atlas.DrawNPatch("panel-base", panel);

    const float pad = 32.0f;
    Rectangle titleRect = {panel.x, panel.y + pad, panel.width, 44.0f};
    LabelCentered(ui, "HOW TO PLAY", titleRect, 34.0f, Ink, true);

    const std::vector<std::string>& lines = data::Rules().tutorialLines;
    const float bullet = 6.0f;
    const float indent = 20.0f;
    const float fontSize = 22.0f;
    float lineW = panel.width - pad * 2.0f - indent;
    float y = panel.y + pad + 66.0f;
    for (const std::string& line : lines)
    {
        ui.Theme().Fill(Rectangle{panel.x + pad, y + fontSize * 0.45f, bullet, bullet}, Dim);
        float h = DrawRichText(ui, line, Rectangle{panel.x + pad + indent, y, lineW, fontSize}, fontSize, Ink, WHITE,
                               true);
        y += h + 14.0f;
    }

    Rectangle ok = HelpOkButton();
    bool okHover = in.Hover(ok);
    bool okDown = okHover && in.Down();
    Color okTint = okDown ? Color{56, 62, 48, 255} : (okHover ? Color{96, 106, 82, 255} : Color{70, 78, 60, 255});
    atlas.DrawNPatch("panel-base", ok, okTint);
    LabelCentered(ui, "OK", ok, 24.0f, Ink, true);
}
}
