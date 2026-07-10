#ifndef DEBUG_UI_H
#define DEBUG_UI_H

#include <functional>

namespace view { class Renderer; }

namespace debug {

void DebugUiSetup();
void DebugUiShutdown();
void DrawDebugOverlay(float& cameraBoundsRadius, view::Renderer& renderer,
                      const std::function<void()>& onResetResource);

}

#endif
