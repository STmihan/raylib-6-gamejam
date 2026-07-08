#ifndef VIEW_DEBUG_UI_H
#define VIEW_DEBUG_UI_H

#include "view/water_params.h"

namespace view {

void DebugUiSetup();
void DebugUiShutdown();
void DrawDebugOverlay(WaterParams &water);

}

#endif
