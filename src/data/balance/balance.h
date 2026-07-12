#ifndef DATA_BALANCE_BALANCE_H
#define DATA_BALANCE_BALANCE_H

#include <array>

#include "data/card/deck_config.h"
#include "data/unit/unit_types.h"

namespace data
{
struct Balance
{
    std::array<UnitStats, UnitTypeCount> units;
    std::array<int, UnitTypeCount> cardCost;
    std::array<int, UnitTypeCount> cardCharges;
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
        {60, 55.0f, true, false, false, false, 15.0f, 2, 1.0f, 15, false, 0};
    b.units[static_cast<int>(UnitType::Rocketeer)] =
        {50, 50.0f, true, false, false, true, 15.0f, 4, 1.5f, 15, false, 0};
    b.units[static_cast<int>(UnitType::Engineer)] =
        {70, 50.0f, true, false, false, false, 15.0f, 1, 1.0f, 0, false, 0};
    b.units[static_cast<int>(UnitType::AA)] =
        {140, 40.0f, false, false, true, true, 55.0f, 3, 0.8f, 20, true, 0};
    b.units[static_cast<int>(UnitType::Tank)] =
        {260, 45.0f, false, false, true, true, 60.0f, 2, 1.3f, 40, false, 3};
    b.units[static_cast<int>(UnitType::Plane)] =
        {120, 90.0f, false, true, true, true, 0.0f, 1, 1.0f, 30, false, 0};

    b.cardCost = {1, 2, 2, 3, 4, 5};
    b.cardCharges = {3, 2, 1, 1, 1, 1};

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
    b.baseRegenPerSec = 0.5f;
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
        {UnitType::AA, 2},
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
