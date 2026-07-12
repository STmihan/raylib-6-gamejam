#include "data/balance/balance.h"

#include <fstream>
#include <string>

#include <nlohmann/json.hpp>

namespace data
{
namespace
{
using json = nlohmann::json;

int UnitIndex(const std::string& name)
{
    if (name == "Infantry") return static_cast<int>(UnitType::Infantry);
    if (name == "Rocketeer") return static_cast<int>(UnitType::Rocketeer);
    if (name == "Engineer") return static_cast<int>(UnitType::Engineer);
    if (name == "AA") return static_cast<int>(UnitType::AA);
    if (name == "Tank") return static_cast<int>(UnitType::Tank);
    if (name == "Plane") return static_cast<int>(UnitType::Plane);
    return -1;
}

void ApplyUnit(Balance& b, int idx, const json& u)
{
    UnitStats& s = b.units[static_cast<std::size_t>(idx)];
    s.hp = u.value("hp", s.hp);
    s.moveSpeed = u.value("moveSpeed", s.moveSpeed);
    s.ignoresSlow = u.value("ignoresSlow", s.ignoresSlow);
    s.ignoresAllTileMods = u.value("ignoresAllTileMods", s.ignoresAllTileMods);
    s.isVehicle = u.value("isVehicle", s.isVehicle);
    s.canTargetAir = u.value("canTargetAir", s.canTargetAir);
    s.footprint = u.value("footprint", s.footprint);
    s.attackRange = u.value("attackRange", s.attackRange);
    s.attackInterval = u.value("attackInterval", s.attackInterval);
    s.baseDamage = u.value("baseDamage", s.baseDamage);
    s.stationary = u.value("stationary", s.stationary);
    s.armorHits = u.value("armorHits", s.armorHits);
    s.aoeRadius = u.value("aoeRadius", s.aoeRadius);
    s.aggroRange = u.value("aggroRange", s.aggroRange);

    b.cardCost[static_cast<std::size_t>(idx)] = u.value("cost", b.cardCost[static_cast<std::size_t>(idx)]);
    b.cardCharges[static_cast<std::size_t>(idx)] =
        u.value("charges", b.cardCharges[static_cast<std::size_t>(idx)]);
    if (u.contains("description") && u["description"].is_array())
    {
        std::vector<std::string> points;
        for (const auto& p : u["description"]) points.push_back(p.get<std::string>());
        b.cardDescription[static_cast<std::size_t>(idx)] = points;
    }
    b.cardMergeGrant[static_cast<std::size_t>(idx)] =
        u.value("mergeGrant", b.cardMergeGrant[static_cast<std::size_t>(idx)]);
}
}

void LoadRules(const char* path)
{
    std::ifstream file(path);
    if (!file) return;

    json root;
    try
    {
        file >> root;
    }
    catch (const json::exception&)
    {
        return;
    }

    Balance b = DefaultBalance();

    const json units = root.value("units", json::object());
    for (auto it = units.begin(); it != units.end(); ++it)
    {
        int idx = UnitIndex(it.key());
        if (idx >= 0 && it.value().is_object()) ApplyUnit(b, idx, it.value());
    }

    const json matrix = root.value("damageMatrix", json::object());
    for (auto atk = matrix.begin(); atk != matrix.end(); ++atk)
    {
        int a = UnitIndex(atk.key());
        if (a < 0 || !atk.value().is_object()) continue;
        for (auto def = atk.value().begin(); def != atk.value().end(); ++def)
        {
            int d = UnitIndex(def.key());
            if (d >= 0) b.damageMatrix[static_cast<std::size_t>(a)][static_cast<std::size_t>(d)] = def.value();
        }
    }
    b.structureDamageMultiplier = root.value("structureDamageMultiplier", b.structureDamageMultiplier);

    const json e = root.value("economy", json::object());
    b.resourceCap = e.value("resourceCap", b.resourceCap);
    b.baseRegenPerSec = e.value("baseRegenPerSec", b.baseRegenPerSec);
    b.deployFreezeSeconds = e.value("deployFreezeSeconds", b.deployFreezeSeconds);
    b.engineerHealDeployRadius = e.value("engineerHealDeployRadius", b.engineerHealDeployRadius);
    b.engineerHealDeployFraction = e.value("engineerHealDeployFraction", b.engineerHealDeployFraction);
    b.engineerHealPulseRadius = e.value("engineerHealPulseRadius", b.engineerHealPulseRadius);
    b.engineerHealPulseAmount = e.value("engineerHealPulseAmount", b.engineerHealPulseAmount);
    b.engineerHealPulseInterval = e.value("engineerHealPulseInterval", b.engineerHealPulseInterval);
    b.baseTurretRange = e.value("baseTurretRange", b.baseTurretRange);
    b.baseTurretDamage = e.value("baseTurretDamage", b.baseTurretDamage);
    b.baseTurretInterval = e.value("baseTurretInterval", b.baseTurretInterval);
    b.matchDurationSeconds = e.value("matchDurationSeconds", b.matchDurationSeconds);
    b.regenStepSeconds = e.value("regenStepSeconds", b.regenStepSeconds);
    b.maxRegenDoublings = e.value("maxRegenDoublings", b.maxRegenDoublings);
    b.baseHp = e.value("baseHp", b.baseHp);
    b.wallHp = e.value("wallHp", b.wallHp);

    const json ot = root.value("overtime", json::object());
    b.overtimeBaseDamage = ot.value("baseDamage", b.overtimeBaseDamage);
    b.overtimeDoubleEverySeconds = ot.value("doubleEverySeconds", b.overtimeDoubleEverySeconds);

    const json ai = root.value("ai", json::object());
    b.aiDeployCooldownSeconds = ai.value("deployCooldownSeconds", b.aiDeployCooldownSeconds);

    b.forestMissPercent = root.value("forestMissPercent", b.forestMissPercent);

    if (root.contains("deck") && root["deck"].is_array())
    {
        int n = 0;
        int total = 0;
        for (const json& item : root["deck"])
        {
            if (n >= MaxDeckEntries) break;
            int idx = UnitIndex(item.value("type", std::string()));
            if (idx < 0) continue;
            int count = item.value("count", 1);
            if (total + count > MaxDeckCards) count = MaxDeckCards - total;
            if (count <= 0) continue;
            b.deck[static_cast<std::size_t>(n)] = {static_cast<UnitType>(idx), count};
            total += count;
            n++;
        }
        if (n > 0) b.deckEntryCount = n;
    }

    SetRules(b);
}
}
