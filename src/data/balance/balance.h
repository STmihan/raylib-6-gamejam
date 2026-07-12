#ifndef DATA_BALANCE_BALANCE_H
#define DATA_BALANCE_BALANCE_H

#include <array>
#include <string>
#include <vector>

#include "data/card/deck_config.h"
#include "data/unit/unit_types.h"

namespace data
{
struct Balance
{
    std::array<UnitStats, UnitTypeCount> units;
    std::array<int, UnitTypeCount> cardCost;
    std::array<int, UnitTypeCount> cardCharges;
    std::array<std::vector<std::string>, UnitTypeCount> cardDescription;
    std::array<std::string, UnitTypeCount> cardMergeGrant;
    std::array<std::array<float, UnitTypeCount>, UnitTypeCount> damageMatrix;
    float structureDamageMultiplier;

    float resourceCap;
    float baseRegenPerSec;
    float deployFreezeSeconds;
    int engineerHealDeployRadius;
    float engineerHealDeployFraction;
    int engineerHealPulseRadius;
    int engineerHealPulseAmount;
    float engineerHealPulseInterval;
    int baseTurretRange;
    int baseTurretDamage;
    float baseTurretInterval;
    int matchDurationSeconds;
    int regenStepSeconds;
    int maxRegenDoublings;
    int overtimeBaseDamage;
    int overtimeDoubleEverySeconds;
    int forestMissPercent;
    int baseHp;
    int wallHp;
    float aiDeployCooldownSeconds;

    std::array<DeckEntry, MaxDeckEntries> deck;
    int deckEntryCount;
};

inline Balance DefaultBalance()
{
    Balance b{};
    b.units[static_cast<int>(UnitType::Infantry)] =
        {60, 110.0f, true, false, false, false, 15.0f, 2, 1.0f, 15, false, 0, 0, 4};
    b.units[static_cast<int>(UnitType::Rocketeer)] =
        {50, 100.0f, true, false, false, true, 15.0f, 4, 1.5f, 22, false, 0, 1, 5};
    b.units[static_cast<int>(UnitType::Engineer)] =
        {50, 100.0f, true, false, false, false, 15.0f, 1, 1.0f, 0, false, 0, 0, 1};
    b.units[static_cast<int>(UnitType::RL)] =
        {140, 80.0f, false, false, true, true, 55.0f, 5, 0.8f, 20, true, 0, 0, 8};
    b.units[static_cast<int>(UnitType::Tank)] =
        {260, 100.0f, false, false, true, true, 60.0f, 2, 1.1f, 40, false, 3, 0, 4};
    b.units[static_cast<int>(UnitType::Plane)] =
        {120, 180.0f, false, true, true, true, 0.0f, 1, 1.0f, 30, false, 0, 0, 3};

    b.cardCost = {1, 2, 2, 3, 4, 5};
    b.cardCharges = {3, 2, 1, 1, 1, 1};

    b.cardDescription[static_cast<int>(UnitType::Infantry)] = {
        "Cheap foot soldier.",
        "Good vs infantry.",
        "Weak to vehicles.",
    };
    b.cardDescription[static_cast<int>(UnitType::Rocketeer)] = {
        "Kills armor.",
        "Long [stat-range] range.",
        "Rockets hit an area.",
    };
    b.cardDescription[static_cast<int>(UnitType::Engineer)] = {
        "No [stat-attack] damage.",
        "Heals vehicles.",
    };
    b.cardDescription[static_cast<int>(UnitType::RL)] = {
        "Hits planes.",
        "Long [stat-range] range.",
        "Can move anywhere.",
    };
    b.cardDescription[static_cast<int>(UnitType::Tank)] = {
        "Strong [stat-shield] armor.",
        "Crushes ground units.",
        "Weak to rockets.",
    };
    b.cardDescription[static_cast<int>(UnitType::Plane)] = {
        "Fast [stat-ms] flyer.",
        "Attacks the base.",
        "Flies over terrain.",
    };

    b.cardMergeGrant[static_cast<int>(UnitType::Infantry)] = "+1 [charge-fill] charge.";
    b.cardMergeGrant[static_cast<int>(UnitType::Rocketeer)] = "More [stat-range] range.";
    b.cardMergeGrant[static_cast<int>(UnitType::Engineer)] = "Heals when placed.";
    b.cardMergeGrant[static_cast<int>(UnitType::RL)] = "Redeploys anywhere, more [stat-range] range.";
    b.cardMergeGrant[static_cast<int>(UnitType::Tank)] = "Stronger [stat-shield] armor.";
    b.cardMergeGrant[static_cast<int>(UnitType::Plane)] = "Place it anywhere.";

    b.damageMatrix = {{
        {1.00f, 1.00f, 1.00f, 0.30f, 0.30f, 0.00f},
        {0.30f, 0.30f, 0.30f, 1.00f, 1.00f, 0.80f},
        {0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f},
        {0.10f, 0.10f, 0.50f, 0.50f, 0.50f, 1.00f},
        {1.50f, 1.00f, 1.00f, 1.00f, 1.00f, 0.30f},
        {0.50f, 0.50f, 1.00f, 1.20f, 1.20f, 1.00f},
    }};
    b.structureDamageMultiplier = 1.0f;

    b.resourceCap = 10.0f;
    b.baseRegenPerSec = 0.375f;
    b.deployFreezeSeconds = 1.0f;
    b.engineerHealDeployRadius = 2;
    b.engineerHealDeployFraction = 0.4f;
    b.engineerHealPulseRadius = 1;
    b.engineerHealPulseAmount = 10;
    b.engineerHealPulseInterval = 2.0f;
    b.baseTurretRange = 4;
    b.baseTurretDamage = 25;
    b.baseTurretInterval = 0.8f;
    b.matchDurationSeconds = 120;
    b.regenStepSeconds = 60;
    b.maxRegenDoublings = 2;
    b.overtimeBaseDamage = 20;
    b.overtimeDoubleEverySeconds = 30;
    b.forestMissPercent = 30;
    b.baseHp = 1500;
    b.wallHp = 75;
    b.aiDeployCooldownSeconds = 3.0f;

    b.deck = {{
        {UnitType::Infantry, 2},
        {UnitType::Rocketeer, 2},
        {UnitType::Engineer, 2},
        {UnitType::RL, 2},
        {UnitType::Tank, 2},
        {UnitType::Plane, 2},
    }};
    b.deckEntryCount = 6;
    return b;
}

inline Balance g_rules = DefaultBalance();

inline const Balance& Rules() { return g_rules; }
inline void SetRules(const Balance& balance) { g_rules = balance; }

void LoadRules(const char* path);
}

#endif
