#!/usr/bin/env python3
"""Normalize fleet data layout and schema.

Actions:
1. Remove deprecated `sourceReliability` from all vehicle type JSON files.
2. Move vehicle instance files to typed subdirectories under data/vehicles/.
3. Ensure every train JSON has required `trainCategory` and move to category
   subdirectories under data/trains/.

The script is idempotent and safe to run multiple times.
"""

from __future__ import annotations

import json
import shutil
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DATA = ROOT / "data"
VEHICLE_TYPES_DIR = DATA / "vehicle_types"
VEHICLES_DIR = DATA / "vehicles"
TRAINS_DIR = DATA / "trains"

VEHICLE_TYPE_TO_DIR = {
    "LOCOMOTIVE": "locomotive",
    "EMU_UNIT": "emu_unit",
    "DMU_UNIT": "dmu_unit",
    "FREIGHT_WAGON": "freight_wagon",
    "PASSENGER_WAGON": "passenger_wagon",
    "SERVICE_WAGON": "service_wagon",
}

TRAIN_CATEGORY_TO_DIR = {
    "PASSENGER": "passenger",
    "FREIGHT": "freight",
    "MAINTENANCE": "shunting",
}

TRAIN_DIR_HINTS = {
    "passenger": "PASSENGER",
    "local": "PASSENGER",
    "freight": "FREIGHT",
    "shunting": "MAINTENANCE",
}


@dataclass
class Stats:
    vehicle_types_scanned: int = 0
    source_reliability_removed: int = 0
    vehicles_scanned: int = 0
    vehicles_moved: int = 0
    trains_scanned: int = 0
    trains_moved: int = 0
    train_category_added_or_fixed: int = 0


def iter_json_files(root: Path):
    for path in sorted(root.rglob("*.json")):
        if path.name == ".gitkeep":
            continue
        yield path


def read_json(path: Path) -> dict:
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def write_json(path: Path, data: dict) -> None:
    with path.open("w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, ensure_ascii=False)
        f.write("\n")


def type_dir_for_vehicle_type(vehicle_type: str) -> str:
    return VEHICLE_TYPE_TO_DIR.get(vehicle_type, "unknown")


def category_from_train_metadata(path: Path, data: dict) -> str:
    # 1) Honor explicit category if valid.
    explicit = str(data.get("trainCategory") or "").upper()
    if explicit in TRAIN_CATEGORY_TO_DIR:
        return explicit

    # 2) Infer from existing parent directories.
    for part in path.parts:
        part_l = part.lower()
        if part_l in TRAIN_DIR_HINTS:
            return TRAIN_DIR_HINTS[part_l]

    # 3) Infer from textual hints.
    hint_text = " ".join(
        [
            str(data.get("pID") or ""),
            str(data.get("displayName") or ""),
            path.stem,
        ]
    ).lower()
    if any(k in hint_text for k in ("tow", "freight", "cargo", "goods")):
        return "FREIGHT"
    if any(k in hint_text for k in ("manewr", "shunt", "maintenance", "rescue")):
        return "MAINTENANCE"

    # 4) Default to passenger.
    return "PASSENGER"


def load_vehicle_type_index() -> dict[str, str]:
    """Map typeID -> vehicleType using all vehicle type JSON files."""
    index: dict[str, str] = {}
    for path in iter_json_files(VEHICLE_TYPES_DIR):
        data = read_json(path)
        type_id = str(data.get("typeID") or "")
        vehicle_type = str(data.get("vehicleType") or "")
        if type_id:
            index[type_id] = vehicle_type
    return index


def ensure_train_dirs() -> None:
    for name in ("passenger", "freight", "local", "shunting"):
        (TRAINS_DIR / name).mkdir(parents=True, exist_ok=True)


def migrate() -> Stats:
    stats = Stats()

    # --- Vehicle types: remove sourceReliability ---
    for path in iter_json_files(VEHICLE_TYPES_DIR):
        stats.vehicle_types_scanned += 1
        data = read_json(path)
        if "sourceReliability" in data:
            del data["sourceReliability"]
            write_json(path, data)
            stats.source_reliability_removed += 1

    # Build type lookup after normalization.
    type_index = load_vehicle_type_index()

    # --- Vehicles: move to typed subdirectories ---
    for path in iter_json_files(VEHICLES_DIR):
        stats.vehicles_scanned += 1
        data = read_json(path)
        type_id = str(data.get("typeID") or "")
        vehicle_type = type_index.get(type_id, "")
        subdir = type_dir_for_vehicle_type(vehicle_type)
        target_dir = VEHICLES_DIR / subdir
        target_dir.mkdir(parents=True, exist_ok=True)
        target = target_dir / path.name

        if path.resolve() != target.resolve():
            shutil.move(str(path), str(target))
            stats.vehicles_moved += 1

    # --- Trains: ensure trainCategory + move to category subdirectories ---
    ensure_train_dirs()
    for path in iter_json_files(TRAINS_DIR):
        stats.trains_scanned += 1
        data = read_json(path)
        category = category_from_train_metadata(path, data)

        current_category = str(data.get("trainCategory") or "").upper()
        if current_category != category:
            data["trainCategory"] = category
            write_json(path, data)
            stats.train_category_added_or_fixed += 1

        target_dir = TRAINS_DIR / TRAIN_CATEGORY_TO_DIR[category]
        target_dir.mkdir(parents=True, exist_ok=True)
        target = target_dir / path.name
        if path.resolve() != target.resolve():
            shutil.move(str(path), str(target))
            stats.trains_moved += 1

    return stats


def main() -> None:
    stats = migrate()
    print("Fleet layout migration complete")
    print(f"  vehicle types scanned: {stats.vehicle_types_scanned}")
    print(f"  sourceReliability removed: {stats.source_reliability_removed}")
    print(f"  vehicles scanned: {stats.vehicles_scanned}")
    print(f"  vehicles moved: {stats.vehicles_moved}")
    print(f"  trains scanned: {stats.trains_scanned}")
    print(f"  trainCategory added/fixed: {stats.train_category_added_or_fixed}")
    print(f"  trains moved: {stats.trains_moved}")


if __name__ == "__main__":
    main()
