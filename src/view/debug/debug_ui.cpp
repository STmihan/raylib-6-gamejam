#include "view/debug/debug_ui.h"

#if defined(HAS_IMGUI) && defined(DEBUG_BUILD)

#include <cstdio>

#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"

#include "data/render/water_params.h"
#include "view/prefab/water.h"

namespace view
{
namespace
{
    void CopyParamsToClipboard(const data::WaterParams& p)
    {
        char buffer[1024];
        std::snprintf(buffer, sizeof(buffer),
            "deep = {%.3ff, %.3ff, %.3ff};\n"
            "shallow = {%.3ff, %.3ff, %.3ff};\n"
            "foam = {%.3ff, %.3ff, %.3ff};\n"
            "outline = {%.3ff, %.3ff, %.3ff};\n"
            "colorRange = %.3ff;\n"
            "foamDistance = %.3ff;\n"
            "foamCutoff = %.3ff;\n"
            "noiseScale = %.3ff;\n"
            "distortAmount = %.3ff;\n"
            "scrollSpeed = %.3ff;\n"
            "outlineWidth = %.3ff;\n"
            "flowSpeed = %.3ff;\n"
            "flowAmount = %.3ff;\n"
            "lineThick = %.3ff;\n"
            "lineGap = %.3ff;\n"
            "lineThin = %.3ff;\n"
            "lineTravel = %.3ff;\n"
            "lineSpeed = %.3ff;\n"
            "lineInterval = %.3ff;\n"
            "lineWobble = %.3ff;\n"
            "lineWobbleScale = %.3ff;\n"
            "lineWobbleSpeed = %.3ff;\n"
            "detailAmount = %.3ff;\n"
            "detailScale = %.3ff;\n"
            "detailSpeed = %.3ff;\n"
            "detailReach = %.3ff;\n",
            p.deep.x, p.deep.y, p.deep.z,
            p.shallow.x, p.shallow.y, p.shallow.z,
            p.foam.x, p.foam.y, p.foam.z,
            p.outline.x, p.outline.y, p.outline.z,
            p.colorRange, p.foamDistance, p.foamCutoff, p.noiseScale,
            p.distortAmount, p.scrollSpeed, p.outlineWidth, p.flowSpeed, p.flowAmount,
            p.lineThick, p.lineGap, p.lineThin, p.lineTravel, p.lineSpeed, p.lineInterval,
            p.lineWobble, p.lineWobbleScale, p.lineWobbleSpeed,
            p.detailAmount, p.detailScale, p.detailSpeed, p.detailReach);
        ImGui::SetClipboardText(buffer);
    }

    void DrawWaterControls(WaterEffect& water)
    {
        data::WaterParams& p = water.ParamsRef();

        const char* modes[] = {"Classic", "Lines"};
        int mode = water.Mode();
        if (ImGui::Combo("Shader", &mode, modes, IM_ARRAYSIZE(modes)))
        {
            water.SetMode(mode);
        }

        ImGui::ColorEdit3("Deep", &p.deep.x);
        ImGui::ColorEdit3("Shallow", &p.shallow.x);
        ImGui::ColorEdit3("Foam", &p.foam.x);
        ImGui::ColorEdit3("Outline", &p.outline.x);
        ImGui::SliderFloat("Color Range", &p.colorRange, 0.5f, 16.0f);
        ImGui::SliderFloat("Outline Width", &p.outlineWidth, 0.0f, 2.0f);

        if (mode == 0)
        {
            ImGui::SeparatorText("Classic");
            ImGui::SliderFloat("Foam Distance", &p.foamDistance, 0.2f, 8.0f);
            ImGui::SliderFloat("Foam Cutoff", &p.foamCutoff, 0.0f, 1.0f);
            ImGui::SliderFloat("Noise Scale", &p.noiseScale, 0.05f, 8.0f);
            ImGui::SliderFloat("Distort Amount", &p.distortAmount, 0.0f, 0.5f);
            ImGui::SliderFloat("Scroll Speed", &p.scrollSpeed, 0.0f, 0.3f);
            ImGui::SliderFloat("Flow Speed", &p.flowSpeed, 0.0f, 4.0f);
            ImGui::SliderFloat("Flow Amount", &p.flowAmount, 0.0f, 0.6f);
        }
        else
        {
            ImGui::SeparatorText("Lines");
            ImGui::SliderFloat("Line Thick", &p.lineThick, 0.02f, 2.0f);
            ImGui::SliderFloat("Line Gap", &p.lineGap, 0.0f, 2.0f);
            ImGui::SliderFloat("Line Thin", &p.lineThin, 0.02f, 1.0f);
            ImGui::SliderFloat("Line Travel", &p.lineTravel, 0.2f, 6.0f);
            ImGui::SliderFloat("Line Speed", &p.lineSpeed, 0.0f, 2.0f);
            ImGui::SliderFloat("Line Interval", &p.lineInterval, 0.1f, 6.0f);
            ImGui::SliderFloat("Line Wobble", &p.lineWobble, 0.0f, 1.0f);
            ImGui::SliderFloat("Wobble Scale", &p.lineWobbleScale, 0.2f, 12.0f);
            ImGui::SliderFloat("Wobble Speed", &p.lineWobbleSpeed, 0.0f, 3.0f);
            ImGui::SliderFloat("Detail Amount", &p.detailAmount, 0.0f, 1.0f);
            ImGui::SliderFloat("Detail Scale", &p.detailScale, 2.0f, 80.0f);
            ImGui::SliderFloat("Detail Speed", &p.detailSpeed, 0.0f, 2.0f);
            ImGui::SliderFloat("Detail Reach", &p.detailReach, 0.2f, 8.0f);
        }

        if (ImGui::Button("Copy params")) CopyParamsToClipboard(p);
    }
}

void DebugUiSetup()
{
    rlImGuiSetup(true);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void DebugUiShutdown()
{
    rlImGuiShutdown();
}

void DrawDebugOverlay(float& cameraBoundsRadius, WaterEffect& water)
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
        if (ImGui::CollapsingHeader("Water", ImGuiTreeNodeFlags_DefaultOpen))
        {
            DrawWaterControls(water);
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

void DrawDebugOverlay(float&, WaterEffect&)
{
}
}

#endif
