#ifndef VIEW_DEBUG_UI_H
#define VIEW_DEBUG_UI_H

namespace view {

class WaterEffect;

void DebugUiSetup();
void DebugUiShutdown();
void DrawDebugOverlay(float& cameraBoundsRadius, WaterEffect& water);

}

#endif
