#include "view/prefab/control_overlay.h"

#include <cmath>
#include <cstddef>

#include "raylib.h"
#include "rlgl.h"

#include "data/space/hex.h"
#include "data/unit/unit.h"
#include "logic/deploy.h"
#include "logic/state/game_state.h"
#include "logic/world/map.h"
#include "view/prefab/registries/model_registry.h"
#include "view/prefab/scene.h"
#include "view/prefab/unit_view.h"
#include "view/space/hex_decal.h"
#include "view/space/world_space.h"

namespace view
{
void ControlOverlayView::SetDrag(bool active, data::Vec2 from, data::Vec2 to, bool movable, bool healer, int range)
{
    active_ = active;
    from_ = from;
    to_ = to;
    movable_ = movable;
    healer_ = healer;
    range_ = range;
}

void ControlOverlayView::Draw(const logic::GameState& previous, const logic::GameState& current, float alpha,
                              float time, const logic::Map& map, const ModelRegistry& models,
                              const UnitView& units, const PlaneOrbitParams& orbit, const bool* occluded) const
{
    if (!active_) return;

    DrawRangeRing(models);

    Vector3 from = LogicToWorld(from_, 0.35f);
    Vector3 to = LogicToWorld(to_, 0.35f);
    Color pointer = {255, 224, 96, 255};
    DrawCylinderEx(from, to, 0.045f, 0.045f, 8, pointer);
    DrawSphere(to, 0.13f, pointer);

    float pulse = 0.5f + 0.5f * std::sin(time * 9.0f);
    auto a = static_cast<unsigned char>(70.0f + 150.0f * pulse);

    if (healer_)
    {
        int ally = logic::PickFriendlyUnit(current, to_, data::PlayerTeam);
        if (ally >= 0)
        {
            Color green = {70, 235, 120, a};
            DrawUnitHighlight(previous, current, alpha, time, map, models, units, orbit, ally, green);
        }
        return;
    }

    int target = logic::PickEnemyTarget(current, to_, data::PlayerTeam, map);
    if (target >= 0)
    {
        Color red = {235, 44, 44, a};
        DrawUnitHighlight(previous, current, alpha, time, map, models, units, orbit, target, red);
    }
    else if (movable_)
    {
        DrawMoveCell(map, models, occluded);
    }
}

void ControlOverlayView::DrawUnitHighlight(const logic::GameState& previous, const logic::GameState& current,
                                           float alpha, float time, const logic::Map& map,
                                           const ModelRegistry& models, const UnitView& units,
                                           const PlaneOrbitParams& orbit, int target, Color tint) const
{
    rlDisableDepthTest();
    BeginBlendMode(BLEND_ALPHA);
    const logic::Entity& e = current.entities[target];
    if (e.kind == logic::EntityKind::Unit)
    {
        units.DrawHighlight(models, previous, current, alpha, orbit, time, target, tint);
    }
    else if (e.kind == logic::EntityKind::Wall)
    {
        DrawModelYaw(models.Wall(), LogicToWorld(e.position, 0.0f), 0.0f, tint);
    }
    else
    {
        Scene scene(models);
        scene.DrawBaseHighlight(map, e.team, tint);
    }
    EndBlendMode();
    rlEnableDepthTest();
}

void ControlOverlayView::DrawRangeRing(const ModelRegistry& models) const
{
    if (range_ <= 0) return;
    data::Offset here = data::CellFromLogic(from_);

    const Color fill = {255, 205, 90, 55};
    const Color line = {255, 220, 120, 220};
    BeginBlendMode(BLEND_ALPHA);
    for (int row = 0; row < logic::MapRows; row++)
    {
        for (int col = 0; col < logic::MapCols; col++)
        {
            if (data::HexDistance(here, {col, row}) > range_) continue;
            DrawModelYaw(models.TileWhite(), LogicToWorld(data::CellToLogic(col, row), 0.07f), 0.0f, fill);
        }
    }
    EndBlendMode();

    for (int row = 0; row < logic::MapRows; row++)
    {
        for (int col = 0; col < logic::MapCols; col++)
        {
            if (data::HexDistance(here, {col, row}) > range_) continue;
            Vector3 c = LogicToWorld(data::CellToLogic(col, row), 0.11f);
            for (int dir = 0; dir < 6; dir++)
            {
                if (data::HexDistance(here, data::Neighbor({col, row}, dir)) <= range_) continue;
                DrawHexEdge(HexCorner(c, HexEdgeCorners[dir][0]), HexCorner(c, HexEdgeCorners[dir][1]), 0.08f, line);
            }
        }
    }
}

void ControlOverlayView::DrawMoveCell(const logic::Map& map, const ModelRegistry& models, const bool* occluded) const
{
    data::Offset cell = data::CellFromLogic(to_);
    if (!map.InBounds(cell.col, cell.row)) return;
    bool occupied = occluded[static_cast<std::size_t>(cell.row) * logic::MapCols + cell.col];
    const Color blueFill = {90, 170, 255, 110};
    const Color blueLine = {110, 200, 255, 255};
    const Color redFill = {255, 110, 100, 150};
    const Color redLine = {255, 70, 60, 255};
    data::Vec2 center = data::CellToLogic(cell.col, cell.row);
    BeginBlendMode(BLEND_ALPHA);
    DrawModelYaw(models.TileWhite(), LogicToWorld(center, 0.12f), 0.0f, occupied ? redFill : blueFill);
    EndBlendMode();
    DrawHexOutline(LogicToWorld(center, 0.16f), 0.09f, occupied ? redLine : blueLine);
}
}
