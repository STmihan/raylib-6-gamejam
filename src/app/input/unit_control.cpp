#include "app/input/unit_control.h"

#include "app/core/app.h"
#include "app/input/cursor.h"
#include "data/space/hex.h"
#include "data/unit/unit.h"
#include "logic/deploy.h"
#include "logic/sim/simulation.h"

namespace app
{
void UnitControl::Update(App& app, Camera3D camera)
{
    if (app.currentState.winner >= 0)
    {
        dragSlot_ = -1;
    }
    else
    {
        data::Vec2 logicPos;
        bool onField = CursorLogic(camera, GetMousePosition(), logicPos);

        if (dragSlot_ < 0)
        {
            Acquire(app, logicPos, onField);
        }
        else if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            int slot = dragSlot_;
            dragSlot_ = -1;
            Command(app, slot, logicPos, onField);
        }
    }

    FeedRenderer(app, camera);
}

void UnitControl::Acquire(App& app, data::Vec2 logicPos, bool onField)
{
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    if (app.renderer.Hand().Dragging()) return;
    if (app.renderer.Ui().Input().CapturesMouse()) return;
    if (!onField) return;

    int slot = logic::PickFriendlyUnit(app.currentState, logicPos, data::PlayerTeam);
    if (slot < 0) return;
    data::UnitType type = app.currentState.entities[slot].type;
    if (data::UnitStatsOf(type).baseDamage <= 0 && type != data::UnitType::Engineer) return;
    dragSlot_ = slot;
}

void UnitControl::Command(App& app, int slot, data::Vec2 logicPos, bool onField)
{
    if (!app.currentState.entities[slot].active) return;
    if (!onField) return;

    if (app.currentState.entities[slot].type == data::UnitType::Engineer)
    {
        int ally = logic::PickFriendlyUnit(app.currentState, logicPos, data::PlayerTeam);
        if (ally >= 0 && ally != slot) logic::Simulation::CommandTarget(app.currentState, slot, ally);
        return;
    }

    int enemy = logic::PickEnemyTarget(app.currentState, logicPos, data::PlayerTeam, app.map);
    if (enemy >= 0)
    {
        logic::Simulation::CommandTarget(app.currentState, slot, enemy);
        return;
    }

    if (!data::UnitStatsOf(app.currentState.entities[slot].type).stationary) return;
    data::Offset cell = data::CellFromLogic(logicPos);
    if (!app.map.InBounds(cell.col, cell.row)) return;
    if (logic::CellHasUnit(app.currentState, cell.col, cell.row)) return;
    logic::Simulation::CommandMove(app.currentState, slot, cell.col, cell.row);
}

void UnitControl::FeedRenderer(App& app, Camera3D camera)
{
    if (dragSlot_ >= 0 && app.currentState.entities[dragSlot_].active)
    {
        const logic::Entity& dragged = app.currentState.entities[dragSlot_];
        data::Vec2 to;
        if (!CursorLogic(camera, GetMousePosition(), to)) to = dragged.position;
        bool movable = data::UnitStatsOf(dragged.type).stationary;
        bool healer = dragged.type == data::UnitType::Engineer;
        app.renderer.ControlOverlay().SetDrag(true, dragged.position, to, movable, healer);
    }
    else
    {
        app.renderer.ControlOverlay().SetDrag(false, {}, {}, false, false);
    }
}
}
