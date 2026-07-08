#include "view/debug_ui.h"

#if defined(HAS_IMGUI)

#include <cstdio>

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

void DrawDebugOverlay(WaterParams& water)
{
    static bool visible = true;
    if (IsKeyPressed(KEY_F1)) visible = !visible;

    rlImGuiBegin();
    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
    if (visible && ImGui::Begin("Debug (F1)"))
    {
        if (ImGui::CollapsingHeader("Water", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::ColorEdit3("Deep", &water.deep.x);
            ImGui::ColorEdit3("Shallow", &water.shallow.x);
            ImGui::ColorEdit3("Foam", &water.foam.x);
            ImGui::ColorEdit3("Outline", &water.outline.x);
            ImGui::SliderFloat("Color Range", &water.colorRange, 0.5f, 16.0f);
            ImGui::SliderFloat("Foam Distance", &water.foamDistance, 0.2f, 8.0f);
            ImGui::SliderFloat("Foam Cutoff", &water.foamCutoff, 0.0f, 1.0f);
            ImGui::SliderFloat("Noise Scale", &water.noiseScale, 0.05f, 8.0f);
            ImGui::SliderFloat("Distort Amount", &water.distortAmount, 0.0f, 0.5f);
            ImGui::SliderFloat("Scroll Speed", &water.scrollSpeed, 0.0f, 0.3f);
            ImGui::SliderFloat("Outline Width", &water.outlineWidth, 0.0f, 2.0f);
            ImGui::SliderFloat("Flow Speed", &water.flowSpeed, 0.0f, 4.0f);
            ImGui::SliderFloat("Flow Amount", &water.flowAmount, 0.0f, 0.6f);

            if (ImGui::Button("Copy params"))
            {
                char buffer[2048];
                snprintf(buffer, sizeof(buffer),
                         "deep = { %.3ff, %.3ff, %.3ff };\n"
                         "shallow = { %.3ff, %.3ff, %.3ff };\n"
                         "foam = { %.3ff, %.3ff, %.3ff };\n"
                         "outline = { %.3ff, %.3ff, %.3ff };\n"
                         "colorRange = %.3ff;\n"
                         "foamDistance = %.3ff;\n"
                         "foamCutoff = %.3ff;\n"
                         "noiseScale = %.3ff;\n"
                         "distortAmount = %.3ff;\n"
                         "scrollSpeed = %.3ff;\n"
                         "outlineWidth = %.3ff;\n"
                         "flowSpeed = %.3ff;\n"
                         "flowAmount = %.3ff;\n",
                         water.deep.x, water.deep.y, water.deep.z,
                         water.shallow.x, water.shallow.y, water.shallow.z,
                         water.foam.x, water.foam.y, water.foam.z,
                         water.outline.x, water.outline.y, water.outline.z,
                         water.colorRange, water.foamDistance, water.foamCutoff,
                         water.noiseScale, water.distortAmount, water.scrollSpeed, water.outlineWidth,
                         water.flowSpeed, water.flowAmount);
                SetClipboardText(buffer);
            }
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

void DrawDebugOverlay(WaterParams&)
{
}
}

#endif
