#ifndef DATA_APP_APP_CONFIG_H
#define DATA_APP_APP_CONFIG_H

namespace data
{
inline constexpr int ScreenWidth = 720;
inline constexpr int ScreenHeight = 720;
inline constexpr const char* WindowTitle = "SWO";
inline constexpr int TargetFps = 60;

// Height of the custom title bar overlaid at the top of the window (desktop
// only; the browser provides its own chrome on web). Top-anchored HUD elements
// add this inset so nothing hides behind the bar.
#if defined(__EMSCRIPTEN__)
inline constexpr float TitleBarHeight = 0.0f;
#else
inline constexpr float TitleBarHeight = 30.0f;
#endif
}

#endif
