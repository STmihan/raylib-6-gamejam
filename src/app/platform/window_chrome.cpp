#include "app/platform/window_chrome.h"

#if !defined(__EMSCRIPTEN__)

#include "raylib.h"

#include "data/app/app_config.h"

namespace app::platform
{
namespace
{
    constexpr float Bar = data::TitleBarHeight;
    constexpr float BtnWidth = 46.0f;

    const Color BarColor = {0x17, 0x18, 0x1a, 0xff};
    const Color MinimizeHover = {0x3a, 0x3b, 0x3e, 0xff}; // grey
    const Color CloseHover = {0xe8, 0x11, 0x23, 0xff};    // red
    const Color Ink = {0xff, 0xff, 0xff, 0xff};           // white glyphs

    bool g_closeRequested = false;
    bool g_dragging = false;
    Vector2 g_grab = {0.0f, 0.0f}; // cursor position within the window at drag start

    bool InRect(Vector2 p, Rectangle r)
    {
        return p.x >= r.x && p.x <= r.x + r.width && p.y >= r.y && p.y <= r.y + r.height;
    }
}

bool CloseRequested() { return g_closeRequested; }

void DrawTitleBar()
{
    const float w = static_cast<float>(GetScreenWidth());
    const Rectangle bar = {0.0f, 0.0f, w, Bar};
    const Rectangle closeBtn = {w - BtnWidth, 0.0f, BtnWidth, Bar};
    const Rectangle minBtn = {w - BtnWidth * 2.0f, 0.0f, BtnWidth, Bar};

    const Vector2 m = GetMousePosition();
    const bool overClose = InRect(m, closeBtn);
    const bool overMin = InRect(m, minBtn);
    const bool overBar = InRect(m, bar) && !overClose && !overMin;

    // Interaction
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (overClose) g_closeRequested = true;
        else if (overMin) MinimizeWindow();
        else if (overBar)
        {
            g_dragging = true;
            g_grab = m;
        }
    }
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) g_dragging = false;
    if (g_dragging)
    {
        // Move the window so the grabbed point stays under the cursor.
        const Vector2 wp = GetWindowPosition();
        SetWindowPosition(static_cast<int>(wp.x + m.x - g_grab.x), static_cast<int>(wp.y + m.y - g_grab.y));
    }

    // Bar + button backgrounds
    DrawRectangleRec(bar, BarColor);
    if (overMin) DrawRectangleRec(minBtn, MinimizeHover);
    if (overClose) DrawRectangleRec(closeBtn, CloseHover);

    // Minimize glyph: a horizontal line
    const float minCx = minBtn.x + minBtn.width * 0.5f;
    const float minCy = minBtn.y + minBtn.height * 0.5f;
    DrawLineEx({minCx - 6.0f, minCy}, {minCx + 6.0f, minCy}, 1.5f, Ink);

    // Close glyph: an X
    const float closeCx = closeBtn.x + closeBtn.width * 0.5f;
    const float closeCy = closeBtn.y + closeBtn.height * 0.5f;
    const float s = 5.5f;
    DrawLineEx({closeCx - s, closeCy - s}, {closeCx + s, closeCy + s}, 1.5f, Ink);
    DrawLineEx({closeCx - s, closeCy + s}, {closeCx + s, closeCy - s}, 1.5f, Ink);
}
}

#else

namespace app::platform
{
void DrawTitleBar() {}
bool CloseRequested() { return false; }
}

#endif
