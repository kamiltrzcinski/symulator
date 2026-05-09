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
    multiple_coupling_backfilled: int = 0
    invalid_multiple_coupling_removed: int = 0
    vehicles_scanned: int = 0
    traction_status_backfilled: int = 0
    invalid_traction_status_removed: int = 0
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


def is_traction_capable(vehicle_type: str, vehicle_subtype: str) -> bool:
    vehicle_type = vehicle_type.upper()
    vehicle_subtype = vehicle_subtype.upper()
    if vehicle_type == "LOCOMOTIVE":
        return True
    if vehicle_type in {"EMU_UNIT", "DMU_UNIT"} and vehicle_subtype == "MOTOR":
        return True
    return False


def normalize_token(value: str) -> str:
    return "".join(ch for ch in value.upper() if ch.isalnum())


NON_MU_LOCOMOTIVE_TOKENS = {
    # Low-power shunters and yard-oriented variants are treated as non-MU in this dataset.
    "LS1000",
    "LS1200",
    "LS150",
    "LS180",
    "LS800",
    "S200",
    "SM03",
    "SM04",
    "SM30",
    "SM31",
    "SM32",
    "SM40",
    "SM41",
    "SM42",
    "SM48",
    "SM60",
    "TEM2",
    "TKH49",
}


def infer_multiple_coupling_capable(data: dict) -> bool | None:
    vehicle_type = str(data.get("vehicleType") or "").upper()
    vehicle_subtype = str(data.get("vehicleSubtype") or "").upper()

    if vehicle_type in {"EMU_UNIT", "DMU_UNIT"} and vehicle_subtype == "MOTOR":
        return True

    if vehicle_type != "LOCOMOTIVE":
        return None

    if vehicle_subtype == "STEAM":
        return False

    type_token = normalize_token(str(data.get("typeName") or ""))
    if type_token in NON_MU_LOCOMOTIVE_TOKENS:
        return False

    power_kw = data.get("powerKW")
    if vehicle_subtype == "DIESEL" and isinstance(power_kw, (int, float)) and power_kw < 1000:
        return False

    return True


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


def load_vehicle_type_index() -> dict[str, tuple[str, str]]:
    """Map typeID -> (vehicleType, vehicleSubtype) using all vehicle type JSON files."""
    index: dict[str, tuple[str, str]] = {}
    for path in iter_json_files(VEHICLE_TYPES_DIR):
        data = read_json(path)
        type_id = str(data.get("typeID") or "")
        vehicle_type = str(data.get("vehicleType") or "")
        vehicle_subtype = str(data.get("vehicleSubtype") or "")
        if type_id:
            index[type_id] = (vehicle_type, vehicle_subtype)
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
        changed = False

        if "sourceReliability" in data:
            del data["sourceReliability"]
            stats.source_reliability_removed += 1
            changed = True

        expected_capability = infer_multiple_coupling_capable(data)
        if expected_capability is None:
            if "multipleCouplingCapable" in data:
                del data["multipleCouplingCapable"]
                stats.invalid_multiple_coupling_removed += 1
                changed = True
        else:
            current_capability = data.get("multipleCouplingCapable")
            if not isinstance(current_capability, bool) or current_capability != expected_capability:
                data["multipleCouplingCapable"] = expected_capability
                stats.multiple_coupling_backfilled += 1
                changed = True

        if changed:
            write_json(path, data)

    # Build type lookup after normalization.
    type_index = load_vehicle_type_index()

    # --- Vehicles: move to typed subdirectories ---
    for path in iter_json_files(VEHICLES_DIR):
        stats.vehicles_scanned += 1
        data = read_json(path)
        type_id = str(data.get("typeID") or "")
        vehicle_type, vehicle_subtype = type_index.get(type_id, ("", ""))

        traction_capable = is_traction_capable(vehicle_type, vehicle_subtype)
        if traction_capable:
            traction_status = str(data.get("tractionStatus") or "").upper()
            if traction_status not in {"OPERATIONAL", "DEFECTIVE"}:
                data["tractionStatus"] = "OPERATIONAL"
                stats.traction_status_backfilled += 1
                write_json(path, data)
        elif "tractionStatus" in data:
            del data["tractionStatus"]
            stats.invalid_traction_status_removed += 1
            write_json(path, data)

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
    print(f"  multipleCouplingCapable backfilled: {stats.multiple_coupling_backfilled}")
    print(f"  invalid multipleCouplingCapable removed: {stats.invalid_multiple_coupling_removed}")
    print(f"  vehicles scanned: {stats.vehicles_scanned}")
    print(f"  tractionStatus backfilled: {stats.traction_status_backfilled}")
    print(f"  invalid tractionStatus removed: {stats.invalid_traction_status_removed}")
    print(f"  vehicles moved: {stats.vehicles_moved}")
    print(f"  trains scanned: {stats.trains_scanned}")
    print(f"  trainCategory added/fixed: {stats.train_category_added_or_fixed}")
    print(f"  trains moved: {stats.trains_moved}")


if __name__ == "__main__":
    main()
