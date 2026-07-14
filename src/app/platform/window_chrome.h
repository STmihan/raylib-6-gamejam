#ifndef APP_PLATFORM_WINDOW_CHROME_H
#define APP_PLATFORM_WINDOW_CHROME_H

namespace app::platform
{
// Draw the custom title bar (dark strip + minimize/close buttons) and handle
// its interaction: dragging the window, minimizing, and requesting close.
// Call once per frame inside BeginDrawing/EndDrawing, on top of everything.
// No-op on web (the browser owns the window chrome).
void DrawTitleBar();

// True once the custom close button has been pressed; the main loop exits.
bool CloseRequested();
}

#endif
