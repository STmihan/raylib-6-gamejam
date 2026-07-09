#ifndef DEBUG_UI_H
#define DEBUG_UI_H

namespace view { class Renderer; }

namespace debug {

void DebugUiSetup();
void DebugUiShutdown();
void DrawDebugOverlay(float& cameraBoundsRadius, view::Renderer& renderer);

}

#endif
