#include "debug/debug_ui.h"

#if defined(HAS_IMGUI) && defined(DEBUG_BUILD)

#include <cstdio>

#include "raylib.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "data/render/shadow_params.h"
#include "data/render/water_params.h"
#include "debug/preview_scene.h"
#include "debug/projectile_test_scene.h"
#include "view/effect/outline_effect.h"
#include "view/prefab/hex_grid.h"
#include "view/prefab/plane_orbit.h"
#include "view/prefab/projectile_view.h"
#include "view/prefab/unit_view.h"
#include "view/prefab/water.h"
#include "view/render/renderer.h"

namespace debug
{
using namespace view;

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

    void CopyGridParams(HexGrid& grid)
    {
        Vector3& c = grid.ColorRef();
        char buffer[256];
        std::snprintf(buffer, sizeof(buffer),
            "color = {%.3ff, %.3ff, %.3ff};\n"
            "thickness = %.3ff;\n"
            "opacity = %.3ff;\n",
            c.x, c.y, c.z, grid.ThicknessRef(), grid.OpacityRef());
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

    void CopyShadowParams(const char* label, const data::ShadowParams& p)
    {
        char buffer[512];
        std::snprintf(buffer, sizeof(buffer),
            "// %s\n"
            "ambient = %.3ff;\n"
            "bands = %.3ff;\n"
            "shadowStrength = %.3ff;\n"
            "softness = %.3ff;\n"
            "biasSlope = %.4ff;\n"
            "biasConstant = %.4ff;\n",
            label, p.ambient, p.bands, p.shadowStrength, p.softness, p.biasSlope, p.biasConstant);
        ImGui::SetClipboardText(buffer);
    }

    void DrawShadowControls(const char* id, data::ShadowParams& p, bool showSunDir)
    {
        ImGui::PushID(id);
        if (showSunDir) ImGui::SliderFloat3("Sun Dir", &p.sunDir.x, -1.0f, 1.0f);
        ImGui::SliderFloat("Ambient", &p.ambient, 0.0f, 1.0f);
        ImGui::SliderFloat("Bands", &p.bands, 1.0f, 8.0f);
        ImGui::SliderFloat("Shadow Strength", &p.shadowStrength, 0.0f, 1.0f);
        ImGui::SliderFloat("Softness", &p.softness, 0.0f, 4.0f);
        ImGui::SliderFloat("Bias Slope", &p.biasSlope, 0.0f, 0.05f, "%.4f");
        ImGui::SliderFloat("Bias Constant", &p.biasConstant, 0.0f, 0.05f, "%.4f");
        if (ImGui::Button("Copy shadow params")) CopyShadowParams(id, p);
        ImGui::PopID();
    }
}

void DebugUiSetup()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    GLFWwindow* window = glfwGetCurrentContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void DebugUiShutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void DrawPreviewMenu(PreviewScene& preview)
{
    ImGui::Begin("Preview (F2)");
    ImGui::TextUnformatted("Right-drag: orbit   Wheel: zoom");
    ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.2f, 1.0f), "Green arrow = front (-Z)");
    ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.0f), "Red arrow = right (+X)");
    ImGui::TextColored(ImVec4(0.9f, 0.85f, 0.2f, 1.0f), "Yellow circle = footprint");
    ImGui::SliderFloat("HP bar", &preview.HpFractionRef(), 0.0f, 1.0f);
    if (ImGui::Button("Hit")) preview.Hit();
    ImGui::Separator();
    for (int i = 0; i < preview.ItemCount(); i++)
    {
        if (ImGui::Selectable(preview.ItemName(i), preview.SelectedRef() == i))
        {
            preview.SelectedRef() = i;
        }
    }
    ImGui::End();
}

void DrawProjectileTestMenu(ProjectileTestScene& projTest)
{
    ImGui::Begin("Projectiles (F4)");
    ImGui::TextUnformatted("Right-drag: orbit   Wheel: zoom");
    ImGui::Separator();
    const char* attackers[] = {
        projTest.AttackerName(0), projTest.AttackerName(1), projTest.AttackerName(2),
        projTest.AttackerName(3), projTest.AttackerName(4), projTest.AttackerName(5),
    };
    ImGui::Combo("Attacker", &projTest.AttackerRef(), attackers, projTest.AttackerCount());
    const char* targets[] = {
        projTest.TargetName(0), projTest.TargetName(1), projTest.TargetName(2),
        projTest.TargetName(3), projTest.TargetName(4), projTest.TargetName(5),
    };
    ImGui::Combo("Target", &projTest.TargetRef(), targets, projTest.TargetCount());
    ImGui::End();
}

void DrawDebugOverlay(float& cameraBoundsRadius, view::Renderer& renderer,
                      const std::function<void()>& onResetResource)
{
    WaterEffect& water = renderer.Water();
    HexGrid& grid = renderer.Grid();
    OutlineEffect& outline = renderer.Outline();
    PreviewScene& preview = renderer.Preview();
    UnitView& units = renderer.Units();
    ProjectileView& projectiles = renderer.Projectiles();
    PlaneOrbitParams& orbit = renderer.OrbitParams();
    ProjectileTestScene& projTest = renderer.ProjTest();
    data::ShadowParams& envShadow = renderer.ShadowParamsRef();
    data::ShadowParams& unitShadow = renderer.UnitShadowParamsRef();

    static bool visible = false;
    if (IsKeyPressed(KEY_F1)) visible = !visible;
    if (IsKeyPressed(KEY_F2))
    {
        preview.ActiveRef() = !preview.Active();
        if (preview.Active()) projTest.ActiveRef() = false;
    }
    if (IsKeyPressed(KEY_F3)) renderer.DragZoneRef() = !renderer.DragZoneRef();
    if (IsKeyPressed(KEY_F4))
    {
        projTest.ActiveRef() = !projTest.Active();
        if (projTest.Active()) preview.ActiveRef() = false;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
    if (preview.Active()) DrawPreviewMenu(preview);
    if (projTest.Active()) DrawProjectileTestMenu(projTest);
    if (visible && ImGui::Begin("Debug (F1)"))
    {
        ImGui::Checkbox("Hide HUD", &renderer.HudHiddenRef());
        ImGui::Checkbox("New UI", &renderer.NewUiRef());
        ImGui::SliderInt("Resource highlight N", &renderer.HudResourceHighlightRef(), 0, 6);
        if (ImGui::Button("Reset player resource") && onResetResource) onResetResource();
        if (ImGui::CollapsingHeader("Resource crystal", ImGuiTreeNodeFlags_DefaultOpen))
        {
            data::CrystalStyle& cs = renderer.CrystalStyleRef();
            ImGui::ColorEdit3("Crystal Top", &cs.top.x);
            ImGui::ColorEdit3("Crystal Bottom", &cs.bottom.x);
            ImGui::SliderFloat("Crystal Gloss", &cs.gloss, 0.0f, 1.0f);
            ImGui::SliderFloat("Crystal Split", &cs.split, 0.0f, 1.0f);
            ImGui::SliderFloat("Crystal Edge", &cs.edge, 0.0f, 0.5f);
            ImGui::ColorEdit3("Crystal Outline", &cs.outline.x);
            ImGui::SliderFloat("Crystal Outline W", &cs.outlineWidth, 0.0f, 4.0f);
        }
        if (ImGui::CollapsingHeader("Hand"))
        {
            data::HandParams& hp = renderer.Hand().ParamsRef();
            ImGui::SliderFloat2("Fan Pivot", &hp.pivot.x, 0.0f, 1200.0f);
            ImGui::SliderFloat("Fan Radius", &hp.radius, 100.0f, 800.0f);
            ImGui::SliderFloat("Fan Step", &hp.step, 0.0f, 30.0f);
            ImGui::SliderFloat("Hover Raise", &hp.raise, 0.0f, 120.0f);
            ImGui::SliderFloat("Hover Scale", &hp.hoverScale, 0.0f, 0.5f);
            ImGui::SliderFloat("Hover Time", &hp.hoverTime, 0.02f, 0.4f);
            ImGui::SliderFloat2("Drag Anchor", &hp.anchor.x, 0.0f, 1200.0f);
            ImGui::SliderFloat("Drag Radius", &hp.dragRadius, 50.0f, 600.0f);
        }
        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("Bounds Radius", &cameraBoundsRadius, 0.0f, 40.0f);
        }
        if (ImGui::CollapsingHeader("Units render", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("Unit Outline", &outline.UnitOutlineScaleRef(), 0.0f, 2.0f);
            ImGui::SliderFloat("Unit Cavity", &outline.UnitCavityScaleRef(), 0.0f, 2.0f);
            ImGui::SliderFloat("Blob Shadow Radius", &units.BlobRadiusRef(), 0.0f, 4.0f);
            ImGui::SliderFloat("Blob Shadow Opacity", &units.BlobOpacityRef(), 0.0f, 1.0f);
        }
        if (ImGui::CollapsingHeader("Shadows - Environment", ImGuiTreeNodeFlags_DefaultOpen))
        {
            DrawShadowControls("env", envShadow, true);
        }
        if (ImGui::CollapsingHeader("Shadows - Units", ImGuiTreeNodeFlags_DefaultOpen))
        {
            DrawShadowControls("units", unitShadow, false);
        }
        if (ImGui::CollapsingHeader("Plane Orbit", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("Air Orbit Radius", &orbit.airRadius, 0.1f, 5.0f);
            ImGui::SliderFloat("Ground Orbit Radius", &orbit.groundRadius, 0.1f, 5.0f);
            ImGui::SliderFloat("Orbit Speed", &orbit.speed, 0.0f, 5.0f);
            ImGui::SliderFloat("Plane Bank", &orbit.bank, -80.0f, 80.0f);
            ImGui::SliderFloat("Plane Altitude", &orbit.altitude, 0.5f, 5.0f);
        }
        if (ImGui::CollapsingHeader("Projectiles", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("Arc Height", &projectiles.ArcHeightRef(), 0.0f, 4.0f);
            ImGui::SliderFloat("Arc Curve", &projectiles.ArcCurveRef(), 0.0f, 5.0f);
            ImGui::SliderFloat("Shell Scale", &projectiles.ShellScaleRef(), 0.1f, 4.0f);
            ImGui::SliderFloat("Launch Height", &projectiles.LaunchHeightRef(), 0.0f, 3.0f);
            ImGui::SliderFloat("Target Height", &projectiles.TargetHeightRef(), 0.0f, 3.0f);
        }
        if (ImGui::CollapsingHeader("Hex Grid", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::ColorEdit3("Line Color", &grid.ColorRef().x);
            ImGui::SliderFloat("Line Thickness", &grid.ThicknessRef(), 0.01f, 0.5f);
            ImGui::SliderFloat("Opacity", &grid.OpacityRef(), 0.0f, 1.0f);
            if (ImGui::Button("Copy grid params")) CopyGridParams(grid);
        }
        if (ImGui::CollapsingHeader("Water", ImGuiTreeNodeFlags_DefaultOpen))
        {
            DrawWaterControls(water);
        }
    }
    if (visible) ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup);
    }
}
}

#else

#include <functional>

namespace view { class Renderer; }

namespace debug
{
void DebugUiSetup()
{
}

void DebugUiShutdown()
{
}

void DrawDebugOverlay(float&, view::Renderer&, const std::function<void()>&)
{
}
}

#endif
