#include "logic/cards/cards.h"

#include "data/card/card.h"
#include "data/card/deck_config.h"
#include "logic/deploy.h"
#include "logic/sim/simulation.h"

namespace logic
{
namespace
{
std::uint32_t MixSeed(std::uint32_t seed, int team)
{
    std::uint32_t s = seed ^ (static_cast<std::uint32_t>(team) * 0x9e3779b9U);
    return s == 0 ? 0x1234567U : s;
}

std::uint32_t NextRandom(std::uint32_t& rng)
{
    rng ^= rng << 13;
    rng ^= rng >> 17;
    rng ^= rng << 5;
    return rng;
}

data::UnitType DrawCard(Deck& deck)
{
    data::UnitType card = deck.cards[static_cast<std::size_t>(deck.head)];
    deck.head = (deck.head + 1) % data::MaxDeckCards;
    deck.count--;
    return card;
}

void ReturnCard(Deck& deck, data::UnitType type)
{
    int tail = (deck.head + deck.count) % data::MaxDeckCards;
    deck.cards[static_cast<std::size_t>(tail)] = type;
    deck.count++;
}

void FillSlot(HandSlot& slot, data::UnitType type)
{
    slot.type = type;
    slot.donor = -1;
    slot.chargesLeft = data::CardDefOf(type).charges;
}

void MarkPlayed(PlayerCards& player, int slot)
{
    HandSlot& s = player.hand[static_cast<std::size_t>(slot)];
    s.chargesLeft--;
    if (s.chargesLeft > 0) return;
    ReturnCard(player.deck, s.type);
    if (s.donor >= 0) ReturnCard(player.deck, static_cast<data::UnitType>(s.donor));
    FillSlot(s, DrawCard(player.deck));
}
}

void InitPlayerCards(GameState& state, std::uint32_t seed)
{
    for (int t = 0; t < 2; t++)
    {
        PlayerCards& player = state.players[static_cast<std::size_t>(t)];
        Deck& deck = player.deck;

        int n = 0;
        for (int e = 0; e < data::DeckEntryCount; e++)
        {
            for (int c = 0; c < data::DeckList[e].count; c++)
            {
                deck.cards[static_cast<std::size_t>(n)] = data::DeckList[e].type;
                n++;
            }
        }

        std::uint32_t rng = MixSeed(seed, t);
        for (int i = n - 1; i > 0; i--)
        {
            int j = static_cast<int>(NextRandom(rng) % static_cast<std::uint32_t>(i + 1));
            data::UnitType tmp = deck.cards[static_cast<std::size_t>(i)];
            deck.cards[static_cast<std::size_t>(i)] = deck.cards[static_cast<std::size_t>(j)];
            deck.cards[static_cast<std::size_t>(j)] = tmp;
        }

        deck.head = 0;
        deck.count = n;

        for (int i = 0; i < data::HandSize; i++)
        {
            FillSlot(player.hand[static_cast<std::size_t>(i)], DrawCard(deck));
        }
    }
}

int SlotCost(const HandSlot& slot)
{
    int cost = data::CardDefOf(slot.type).cost;
    if (slot.donor >= 0) cost += data::CardDefOf(static_cast<data::UnitType>(slot.donor)).cost;
    return cost;
}

bool CanMerge(const PlayerCards& player, int host, int donor)
{
    if (host < 0 || donor < 0 || host == donor) return false;
    if (host >= data::HandSize || donor >= data::HandSize) return false;
    const HandSlot& h = player.hand[static_cast<std::size_t>(host)];
    const HandSlot& d = player.hand[static_cast<std::size_t>(donor)];
    return d.donor < 0 && h.donor < 0 && h.type != d.type;
}

void MergeSlots(GameState& state, data::Team team, int host, int donor)
{
    PlayerCards& player = state.players[static_cast<std::size_t>(data::TeamIndex(team))];
    if (!CanMerge(player, host, donor)) return;

    HandSlot& hostSlot = player.hand[static_cast<std::size_t>(host)];
    HandSlot& donorSlot = player.hand[static_cast<std::size_t>(donor)];
    hostSlot.donor = static_cast<int>(donorSlot.type);
    if (donorSlot.type == data::UnitType::Infantry) hostSlot.chargesLeft += 1;
    FillSlot(donorSlot, DrawCard(player.deck));
}

int PlayCard(GameState& state, const Map& map, data::Team team, int slot, int col, int row)
{
    if (slot < 0 || slot >= data::HandSize) return -1;
    int pidx = data::TeamIndex(team);
    PlayerCards& player = state.players[static_cast<std::size_t>(pidx)];
    HandSlot& s = player.hand[static_cast<std::size_t>(slot)];

    bool anywhere = s.type == data::UnitType::Plane || s.donor == static_cast<int>(data::UnitType::Plane);
    if (!IsDeployable(map, col, row, team, anywhere)) return -1;
    if (CellHasUnit(state, col, row)) return -1;

    int cost = SlotCost(s);
    if (state.resource[static_cast<std::size_t>(pidx)] < static_cast<float>(cost)) return -1;

    int spawned = Simulation::Deploy(state, s.type, s.donor, team, col, row);
    if (spawned < 0) return -1;

    state.resource[static_cast<std::size_t>(pidx)] -= static_cast<float>(cost);
    MarkPlayed(player, slot);
    return spawned;
}
}
