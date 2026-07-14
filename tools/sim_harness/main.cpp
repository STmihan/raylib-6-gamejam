#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <random>
#include <thread>
#include <vector>

#include "ai/policy.h"
#include "data/balance/balance.h"
#include "data/economy/economy.h"
#include "data/time/time_config.h"
#include "data/unit/unit.h"
#include "logic/cards/cards.h"
#include "logic/path.h"
#include "logic/sim/simulation.h"
#include "logic/state/game_state.h"
#include "logic/world/map.h"

namespace
{
int CountActive(const logic::GameState& state)
{
    int n = 0;
    for (int i = 0; i < data::MaxEntities; i++)
    {
        if (state.entities[i].active) n++;
    }
    return n;
}

int BaseHpOf(const logic::GameState& state, data::Team team)
{
    for (int i = 0; i < data::MaxEntities; i++)
    {
        const logic::Entity& e = state.entities[i];
        if (e.active && e.kind == logic::EntityKind::Base && e.team == team) return e.hp;
    }
    return 0;
}

int CountTeamUnits(const logic::GameState& state, data::Team team)
{
    int n = 0;
    for (int i = 0; i < data::MaxEntities; i++)
    {
        const logic::Entity& e = state.entities[i];
        if (e.active && e.kind == logic::EntityKind::Unit && e.team == team) n++;
    }
    return n;
}

void RunMatchVerbose(const logic::Map& map, std::uint32_t seed)
{
    auto state = std::make_unique<logic::GameState>();
    logic::Simulation sim;
    sim.Init(*state, map, seed);
    std::printf("=== match seed=%u ===\n", seed);

    std::uint64_t decisionEvery = static_cast<std::uint64_t>(data::AiDeployCooldownSeconds() * data::TickRate);

    if (decisionEvery < 1) decisionEvery = 1;
    const std::uint64_t snapshotEvery = static_cast<std::uint64_t>(data::TickRate) * 5;
    const std::uint64_t maxTicks = static_cast<std::uint64_t>(data::MatchDurationSeconds() + 180) * data::TickRate;
    data::Team teams[2] = {data::Team::Bottom, data::Team::Top};

    while (state->winner < 0 && state->tick < maxTicks)
    {
        if (state->tick % decisionEvery == 0)
        {
            for (data::Team team : teams)
            {
                ai::Action a = ai::DecideAction(*state, map, team);
                if (a.kind == ai::ActionKind::Play)
                {
                    int pidx = data::TeamIndex(team);
                    const logic::HandSlot& s = state->players[static_cast<std::size_t>(pidx)]
                                                   .hand[static_cast<std::size_t>(a.slot)];
                    std::printf("[%3ds] %-6s PLAY %-9s @(%2d,%2d) cost=%d res=%.1f\n",
                                static_cast<int>(state->tick / data::TickRate),
                                team == data::Team::Top ? "top" : "bottom", data::UnitTypeName(s.type), a.col, a.row,
                                logic::SlotCost(s), state->resource[static_cast<std::size_t>(pidx)]);
                }
                ai::ApplyAction(*state, map, team, a);
                ai::RunMicro(*state, map, team);
            }
        }
        if (state->tick % snapshotEvery == 0)
        {
            std::printf("      snap %3ds  top[u=%2d res=%.1f hp=%4d]  bottom[u=%2d res=%.1f hp=%4d]\n",
                        static_cast<int>(state->tick / data::TickRate),
                        CountTeamUnits(*state, data::Team::Top), state->resource[0],
                        BaseHpOf(*state, data::Team::Top), CountTeamUnits(*state, data::Team::Bottom),
                        state->resource[1], BaseHpOf(*state, data::Team::Bottom));
        }
        sim.Step(*state, static_cast<float>(data::TickDelta));
    }
    std::printf("=== winner=%s tick=%llu (%ds)  topHp=%d bottomHp=%d ===\n",
                state->winner == 0 ? "top" : (state->winner == 1 ? "bottom" : "none"),
                static_cast<unsigned long long>(state->tick), static_cast<int>(state->tick / data::TickRate),
                BaseHpOf(*state, data::Team::Top), BaseHpOf(*state, data::Team::Bottom));
}

struct MatchResult
{
    int winner;
    std::uint64_t ticks;
    std::uint64_t pathCalls;
    std::uint64_t entitySum;
    std::uint64_t entitySamples;
    int topHp;
    int bottomHp;
    bool regularWin;
};

void Decide(logic::GameState& state, const logic::Map& map, data::Team team)
{
    ai::Action action = ai::DecideAction(state, map, team);
    ai::ApplyAction(state, map, team, action);
    ai::RunMicro(state, map, team);
}

MatchResult RunMatch(const logic::Map& map, std::uint32_t seed)
{
    auto state = std::make_unique<logic::GameState>();
    logic::Simulation sim;
    sim.Init(*state, map, seed);
    logic::ResetFindStepCalls();

    std::uint64_t decisionEvery = static_cast<std::uint64_t>(data::AiDeployCooldownSeconds() * data::TickRate);
    if (decisionEvery < 1) decisionEvery = 1;
    const std::uint64_t maxTicks = static_cast<std::uint64_t>(data::MatchDurationSeconds() + 180) * data::TickRate;

    std::uint64_t entitySum = 0;
    std::uint64_t entitySamples = 0;
    while (state->winner < 0 && state->tick < maxTicks)
    {
        if (state->tick % decisionEvery == 0)
        {
            Decide(*state, map, data::Team::Bottom);
            Decide(*state, map, data::Team::Top);
        }
        if (state->tick % 40 == 0)
        {
            entitySum += static_cast<std::uint64_t>(CountActive(*state));
            entitySamples++;
        }
        sim.Step(*state, static_cast<float>(data::TickDelta));
    }

    int elapsedSeconds = static_cast<int>(state->tick / data::TickRate);
    bool regularWin = state->winner >= 0 && elapsedSeconds < data::MatchDurationSeconds();
    return {state->winner, state->tick, logic::FindStepCalls(), entitySum, entitySamples,
            BaseHpOf(*state, data::Team::Top), BaseHpOf(*state, data::Team::Bottom), regularWin};
}

struct Tally
{
    int top = 0;
    int bottom = 0;
    int draws = 0;
    int regularWins = 0;
    std::uint64_t ticks = 0;
    std::uint64_t pathCalls = 0;
    std::uint64_t entitySum = 0;
    std::uint64_t entitySamples = 0;
    std::int64_t topHpSum = 0;
    std::int64_t bottomHpSum = 0;
};

void RunRange(const logic::Map& map, std::uint32_t baseSeed, int from, int to, Tally& out)
{
    for (int i = from; i < to; i++)
    {
        MatchResult r = RunMatch(map, baseSeed + static_cast<std::uint32_t>(i));
        out.ticks += r.ticks;
        out.pathCalls += r.pathCalls;
        out.entitySum += r.entitySum;
        out.entitySamples += r.entitySamples;
        out.topHpSum += r.topHp;
        out.bottomHpSum += r.bottomHp;
        if (r.regularWin) out.regularWins++;
        if (r.winner == 0) out.top++;
        else if (r.winner == 1) out.bottom++;
        else out.draws++;
    }
}
}

int main(int argc, char** argv)
{
    data::LoadRules("assets/config.json");

    int matches = argc > 1 ? std::atoi(argv[1]) : 100;
    if (matches < 1) matches = 1;

    std::uint32_t baseSeed;
    if (argc > 2)
    {
        baseSeed = static_cast<std::uint32_t>(std::strtoul(argv[2], nullptr, 10));
    }
    else
    {
        std::random_device rd;
        baseSeed = rd();
        if (baseSeed == 0) baseSeed = 1;
    }

    int threads = 16;
    if (threads > matches) threads = matches;

    logic::Map map = logic::BuildMap();

    if (matches == 1)
    {
        RunMatchVerbose(map, baseSeed);
        return 0;
    }

    std::vector<Tally> tallies(static_cast<std::size_t>(threads));
    std::vector<std::thread> pool;

    auto start = std::chrono::steady_clock::now();
    for (int t = 0; t < threads; t++)
    {
        int from = static_cast<int>(static_cast<std::int64_t>(matches) * t / threads);
        int to = static_cast<int>(static_cast<std::int64_t>(matches) * (t + 1) / threads);
        pool.emplace_back(RunRange, std::cref(map), baseSeed, from, to,
                          std::ref(tallies[static_cast<std::size_t>(t)]));
    }
    for (std::thread& th : pool) th.join();
    auto end = std::chrono::steady_clock::now();

    Tally total;
    for (const Tally& t : tallies)
    {
        total.top += t.top;
        total.bottom += t.bottom;
        total.draws += t.draws;
        total.regularWins += t.regularWins;
        total.ticks += t.ticks;
        total.pathCalls += t.pathCalls;
        total.entitySum += t.entitySum;
        total.entitySamples += t.entitySamples;
        total.topHpSum += t.topHpSum;
        total.bottomHpSum += t.bottomHpSum;
    }

    double wallSeconds = std::chrono::duration<double>(end - start).count();
    double avgGameSeconds = static_cast<double>(total.ticks) / matches / data::TickRate;
    double matchesPerSec = wallSeconds > 0.0 ? matches / wallSeconds : 0.0;
    double ticksPerSec = wallSeconds > 0.0 ? static_cast<double>(total.ticks) / wallSeconds : 0.0;
    double pathPerMatch = static_cast<double>(total.pathCalls) / matches;
    double pathPerTick = total.ticks > 0 ? static_cast<double>(total.pathCalls) / total.ticks : 0.0;
    double avgEntities = total.entitySamples > 0
        ? static_cast<double>(total.entitySum) / total.entitySamples : 0.0;

    double avgTopHp = static_cast<double>(total.topHpSum) / matches;
    double avgBottomHp = static_cast<double>(total.bottomHpSum) / matches;

    std::printf("matches=%d  threads=%d  seed=%u  top=%d  bottom=%d  draws=%d  regularWins=%d\n", matches, threads,
                baseSeed, total.top, total.bottom, total.draws, total.regularWins);
    std::printf("avgMatchGameTime=%.1fs  totalTicks=%llu  avgEndHp top=%.0f bottom=%.0f\n", avgGameSeconds,
                static_cast<unsigned long long>(total.ticks), avgTopHp, avgBottomHp);
    std::printf("aStar total=%llu  perMatch=%.0f  perTick=%.2f  avgEntities=%.1f\n",
                static_cast<unsigned long long>(total.pathCalls), pathPerMatch, pathPerTick, avgEntities);
    std::printf("wall=%.3fs  %.1f matches/s  %.0f ticks/s\n", wallSeconds, matchesPerSec, ticksPerSec);
    return 0;
}
