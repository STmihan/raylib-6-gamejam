#include "view/debug/debug_ui.h"

#if defined(HAS_IMGUI)

#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"

namespace view
{
void DebugUiSetup()
{
    rlImGuiSetup(true);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void DebugUiShutdown()
{
    rlImGuiShutdown();
}

void DrawDebugOverlay(float& cameraBoundsRadius)
{
    static bool visible = false;
    if (IsKeyPressed(KEY_F1)) visible = !visible;

    rlImGuiBegin();
    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
    if (visible && ImGui::Begin("Debug (F1)"))
    {
        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("Bounds Radius", &cameraBoundsRadius, 0.0f, 40.0f);
        }
    }
    if (visible) ImGui::End();
    rlImGuiEnd();
}
}

#else

namespace view
{
void DebugUiSetup()
{
}

void DebugUiShutdown()
{
}

void DrawDebugOverlay(float&)
{
}
}

#endif
