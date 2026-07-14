import csv
import json
import math
import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CONFIG = os.path.join(ROOT, "assets", "config.json")
OUTPUT = os.path.join(ROOT, "damage_matrix.csv")

ORDER = ["Infantry", "Rocketeer", "Engineer", "RL", "Tank", "Plane"]
DISPLAY = {
    "Infantry": "Infantry",
    "Rocketeer": "Rocketeer",
    "Engineer": "Engineer",
    "RL": "Rocket Launcher",
    "Tank": "Tank",
    "Plane": "Plane",
}


def hits_to_kill(units, matrix, attacker, target):
    damage = int(units[attacker]["baseDamage"] * matrix[attacker][target])
    if damage <= 0:
        return None
    hp = units[target]["hp"]
    armor = units[target]["armorHits"]
    return armor + math.ceil(hp / damage)


def build_table(title, cell_fn):
    header = [title] + [DISPLAY[u] for u in ORDER]
    rows = [header]
    for attacker in ORDER:
        rows.append([DISPLAY[attacker]] + [cell_fn(attacker, t) for t in ORDER])
    rows.append(header)
    return rows


def main():
    with open(CONFIG, encoding="utf-8") as handle:
        config = json.load(handle)

    units = config["units"]
    matrix = config["damageMatrix"]

    def hits_cell(attacker, target):
        hits = hits_to_kill(units, matrix, attacker, target)
        return "-" if hits is None else str(hits)

    def ttk_cell(attacker, target):
        hits = hits_to_kill(units, matrix, attacker, target)
        if hits is None:
            return "-"
        return f"{hits * units[attacker]['attackInterval']:.1f}"

    rows = build_table("Hits to kill", hits_cell)
    rows.append([])
    rows.extend(build_table("Time to kill (s)", ttk_cell))

    with open(OUTPUT, "w", newline="", encoding="utf-8") as handle:
        csv.writer(handle).writerows(rows)

    print("wrote", OUTPUT)


if __name__ == "__main__":
    main()
