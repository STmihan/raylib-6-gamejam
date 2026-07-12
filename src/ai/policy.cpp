#include "ai/policy.h"

#include <climits>
#include <cstddef>

#include "data/economy/economy.h"
#include "data/space/hex.h"
#include "data/tile/tile.h"
#include "data/time/time_config.h"
#include "data/unit/damage.h"
#include "data/unit/unit.h"
#include "logic/cards/cards.h"
#include "logic/deploy.h"
#include "logic/sim/simulation.h"

namespace ai
{
namespace
{
constexpr float DeployThreshold = 0.06f;
constexpr float CounterNeutral = 0.5f;
constexpr float CounterCenter = 0.5f;
constexpr float CounterSpread = 0.45f;
constexpr float DefendRange = 10.0f;
constexpr float OvertimeSupportPenalty = 0.3f;
constexpr float CenterBias = 0.5f;

float Clamp01(float v)
{
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

struct Context
{
    bool valid = false;
    data::Offset ourBase{};
    data::Offset enemyBase{};
    data::Offset threat{};
    bool hasThreat = false;
    float defend = 0.0f;
    bool overtime = false;
    float mapDiag = 1.0f;
    int enemyCount[data::UnitTypeCount] = {0};
    int enemyTotal = 0;
};

Context BuildContext(const logic::GameState& state, data::Team team)
{
    Context ctx;
    ctx.overtime = static_cast<int>(state.tick / data::TickRate) >= data::MatchDurationSeconds();

    bool haveOur = false;
    bool haveEnemy = false;
    for (int i = 0; i < data::MaxEntities; i++)
    {
        const logic::Entity& e = state.entities[i];
        if (!e.active) continue;
        if (e.kind == logic::EntityKind::Base)
        {
            if (e.team == team) { ctx.ourBase = {e.col, e.row}; haveOur = true; }
            else { ctx.enemyBase = {e.col, e.row}; haveEnemy = true; }
        }
        else if (e.kind == logic::EntityKind::Unit && e.team != team)
        {
            ctx.enemyCount[static_cast<int>(e.type)]++;
            ctx.enemyTotal++;
        }
    }

    int bestDist = INT_MAX;
    if (haveOur)
    {
        for (int i = 0; i < data::MaxEntities; i++)
        {
            const logic::Entity& e = state.entities[i];
            if (!e.active || e.kind != logic::EntityKind::Unit || e.team == team) continue;
            int d = data::HexDistance(ctx.ourBase, {e.col, e.row});
            if (d < bestDist) { bestDist = d; ctx.threat = {e.col, e.row}; ctx.hasThreat = true; }
        }
    }

    ctx.mapDiag = haveOur && haveEnemy
        ? static_cast<float>(data::HexDistance(ctx.ourBase, ctx.enemyBase)) : 1.0f;
    if (ctx.mapDiag < 1.0f) ctx.mapDiag = 1.0f;
    ctx.defend = ctx.hasThreat ? Clamp01(1.0f - static_cast<float>(bestDist) / DefendRange) : 0.0f;
    ctx.valid = haveOur && haveEnemy;
    return ctx;
}

float ScoreDeploy(const Context& ctx, data::UnitType type, int col, int row)
{
    float counter;
    if (ctx.enemyTotal == 0)
    {
        counter = CounterNeutral;
    }
    else
    {
        float offense = 0.0f;
        float defense = 0.0f;
        for (int t = 0; t < data::UnitTypeCount; t++)
        {
            if (ctx.enemyCount[t] == 0) continue;
            data::UnitType enemyType = static_cast<data::UnitType>(t);
            float w = static_cast<float>(ctx.enemyCount[t]);
            offense += data::DamageMultiplier(type, enemyType) * w;
            defense += data::DamageMultiplier(enemyType, type) * w;
        }
        offense /= static_cast<float>(ctx.enemyTotal);
        defense /= static_cast<float>(ctx.enemyTotal);
        counter = Clamp01(CounterCenter + CounterSpread * (offense - defense));
    }

    data::Offset here{col, row};
    float toEnemy = static_cast<float>(data::HexDistance(here, ctx.enemyBase));
    float advance = Clamp01((ctx.mapDiag - toEnemy) / (ctx.mapDiag * 0.5f));
    float intercept = 0.0f;
    if (ctx.hasThreat)
    {
        float toThreat = static_cast<float>(data::HexDistance(here, ctx.threat));
        intercept = Clamp01(1.0f - toThreat / (ctx.mapDiag * 0.5f));
    }
    float position = ctx.defend * intercept + (1.0f - ctx.defend) * advance;

    float timing = 1.0f;
    if (ctx.overtime && data::UnitStatsOf(type).baseDamage <= 0) timing = OvertimeSupportPenalty;

    float centerCol = static_cast<float>(logic::MapCols - 1) * 0.5f;
    float lateralOffset = static_cast<float>(col) - centerCol;
    if (lateralOffset < 0.0f) lateralOffset = -lateralOffset;
    float lateral = lateralOffset / (static_cast<float>(logic::MapCols) * 0.5f);
    float center = 1.0f - CenterBias * lateral;

    return counter * position * timing * center;
}

int DefensiveScore(const logic::Map& map, data::Team team, int col, int row)
{
    if (!map.InBounds(col, row)) return 0;
    data::TileType tile = map.At(col, row);
    if (!data::TileConfigOf(tile).passable) return 0;
    bool ownHalf = team == data::Team::Top ? row < logic::MapRows / 2 : row >= logic::MapRows / 2;
    if (!ownHalf) return 0;

    for (int dir = 0; dir < 6; dir++)
    {
        data::Offset nb = data::Neighbor({col, row}, dir);
        if (map.InBounds(nb.col, nb.row) && map.At(nb.col, nb.row) == data::TileType::Empty) return 3;
    }
    if (tile == data::TileType::Forest) return 2;
    int enemyRow = team == data::Team::Top ? row + 1 : row - 1;
    if (map.InBounds(col, enemyRow) && map.At(col, enemyRow) == data::TileType::Wall) return 1;
    return 0;
}
}

Action DecideAction(const logic::GameState& state, const logic::Map& map, data::Team team)
{
    Action best;
    Context ctx = BuildContext(state, team);
    if (!ctx.valid) return best;

    int pidx = data::TeamIndex(team);
    float resource = state.resource[static_cast<std::size_t>(pidx)];
    const logic::PlayerCards& player = state.players[static_cast<std::size_t>(pidx)];

    float bestScore = DeployThreshold;
    for (int slot = 0; slot < data::HandSize; slot++)
    {
        const logic::HandSlot& s = player.hand[static_cast<std::size_t>(slot)];
        if (resource < static_cast<float>(logic::SlotCost(s))) continue;
        bool anywhere = s.type == data::UnitType::Plane || s.donor == static_cast<int>(data::UnitType::Plane);
        for (int row = 0; row < logic::MapRows; row++)
        {
            for (int col = 0; col < logic::MapCols; col++)
            {
                if (!logic::IsDeployable(map, col, row, team, anywhere)) continue;
                if (logic::CellHasUnit(state, col, row)) continue;
                float score = ScoreDeploy(ctx, s.type, col, row);
                if (score > bestScore)
                {
                    bestScore = score;
                    best.kind = ActionKind::Play;
                    best.slot = slot;
                    best.col = col;
                    best.row = row;
                }
            }
        }
    }
    return best;
}

int ApplyAction(logic::GameState& state, const logic::Map& map, data::Team team, const Action& action)
{
    if (action.kind == ActionKind::Play) return logic::PlayCard(state, map, team, action.slot, action.col, action.row);
    if (action.kind == ActionKind::Merge) logic::MergeSlots(state, team, action.host, action.donor);
    return -1;
}

void RunMicro(logic::GameState& state, const logic::Map& map, data::Team team)
{
    for (int i = 0; i < data::MaxEntities; i++)
    {
        logic::Entity& e = state.entities[i];
        if (!e.active || e.kind != logic::EntityKind::Unit || e.team != team) continue;
        if (e.type != data::UnitType::RL || e.deployTimer > 0.0f || e.hasMoveOrder) continue;
        if (DefensiveScore(map, team, e.col, e.row) >= 2) continue;

        int bestScore = 1;
        int bestDist = INT_MAX;
        int destCol = -1;
        int destRow = -1;
        for (int row = 0; row < logic::MapRows; row++)
        {
            for (int col = 0; col < logic::MapCols; col++)
            {
                int score = DefensiveScore(map, team, col, row);
                if (score < 2 || logic::CellHasUnit(state, col, row)) continue;
                int dist = data::HexDistance({e.col, e.row}, {col, row});
                if (score > bestScore || (score == bestScore && dist < bestDist))
                {
                    bestScore = score;
                    bestDist = dist;
                    destCol = col;
                    destRow = row;
                }
            }
        }
        if (destCol >= 0) logic::Simulation::CommandMove(state, i, destCol, destRow);
    }
}
}
