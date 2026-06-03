#!/usr/bin/env python3
"""Migrate all JSON data files from string-based IDs (gID, typeID, sID) to numeric UIDs.

Operates on:
  - data/vehicle_types/**/*.json  : adds "uid" field (ROLLING_STOCK/VEHICLE_TYPE)
  - data/vehicles/**/vehicle.json : "gID"→"uid", "typeID"→"type_uid"
  - data/trains/**/*.json         : "gID"→"uid", "vehicles"→"vehicle_uids"
  - scenarios/*/meta.json         : "station_sid"→"station_uid"
  - scenarios/*/topology.json     : "gID"→"uid", "sID" removed, "neighborID"→"neighborUID", etc.
  - scenarios/*/objects.json      : "gID"→"uid", "sID" removed, references updated

Writes changes IN-PLACE (backs up original files as .bak if --backup is passed).
Run from repo root.

Usage:
  python3 scripts/migrate_uid_data.py [--backup] [--dry-run]
"""

import argparse
import json
import os
import sys
from pathlib import Path

ROOT = Path(__file__).parent.parent

# ── UID bit manipulation ─────────────────────────────────────────────────────

DOMAIN_RS    = 0x01
DOMAIN_INFRA = 0x02
DOMAIN_OPS   = 0x03

KIND_VEHICLE_TYPE  = 0x01
KIND_VEHICLE       = 0x02
KIND_TRAIN_CONSIST = 0x03
KIND_CARRIER       = 0x04

KIND_STATION       = 0x11
KIND_DISPATCH_AREA = 0x12
KIND_TRACK_SECTION = 0x13
KIND_SWITCH        = 0x14
KIND_SIGNAL        = 0x15
KIND_DERAILER      = 0x16
KIND_BLOCK_SECTION = 0x17
KIND_BOUNDARY_NODE = 0x18
KIND_LEVEL_CROSSING= 0x19
KIND_AXLE_COUNTER  = 0x1A
KIND_INTERLOCKING  = 0x1B
KIND_POWER_SUPPLY  = 0x1C


def make_uid(domain: int, kind: int, scope: int, instance: int) -> int:
    assert 1 <= instance <= 0xFFFF, f"instance {instance} out of range"
    assert 0 <= scope <= 0xFFFF, f"scope {scope} out of range"
    return (domain << 40) | (kind << 32) | (scope << 16) | instance


# ── Station registry ──────────────────────────────────────────────────────────

def load_stations() -> dict[str, int]:
    """Returns code → instance mapping."""
    path = ROOT / "data" / "stations.json"
    with open(path) as f:
        data = json.load(f)
    return {entry["code"]: entry["instance"] for entry in data}


# ── Vehicle types ─────────────────────────────────────────────────────────────

def migrate_vehicle_types(dry_run: bool, backup: bool) -> dict[str, int]:
    """Assign UIDs to all vehicle type files. Returns typeID_string → uid mapping."""
    types_dir = ROOT / "data" / "vehicle_types"
    files = sorted(types_dir.rglob("*.json"))

    type_id_to_uid: dict[str, int] = {}
    instance = 0

    for path in files:
        with open(path) as f:
            obj = json.load(f)

        type_id = obj.get("typeID")
        if type_id is None:
            print(f"  WARN: no typeID in {path.relative_to(ROOT)}, skipping")
            continue

        if type_id in type_id_to_uid:
            print(f"  WARN: duplicate typeID {type_id} in {path.relative_to(ROOT)}")
            continue

        instance += 1
        uid = make_uid(DOMAIN_RS, KIND_VEHICLE_TYPE, 0, instance)
        type_id_to_uid[type_id] = uid

        # Build new object: add uid, remove typeID (keep for reference during transition)
        new_obj = {"uid": uid}
        for k, v in obj.items():
            if k != "typeID":
                new_obj[k] = v

        if not dry_run:
            if backup:
                path.rename(path.with_suffix(".json.bak"))
            with open(path, "w", encoding="utf-8") as f:
                json.dump(new_obj, f, indent=2, ensure_ascii=False)
                f.write("\n")
        print(f"  VehicleType [{instance:4d}] {path.relative_to(ROOT)} → uid={uid:#x}")

    print(f"  → {len(type_id_to_uid)} vehicle types assigned UIDs")
    return type_id_to_uid


# ── Vehicles ──────────────────────────────────────────────────────────────────

def migrate_vehicles(type_id_to_uid: dict[str, int], dry_run: bool, backup: bool) -> dict[str, int]:
    """Assign UIDs to all vehicle files. Returns gID_string → uid mapping."""
    vehicles_dir = ROOT / "data" / "vehicles"
    files = sorted(vehicles_dir.rglob("vehicle.json"))

    gid_to_uid: dict[str, int] = {}
    instance = 0

    for path in files:
        with open(path) as f:
            obj = json.load(f)

        gid = obj.get("gID")
        type_id = obj.get("typeID")

        if gid is None:
            print(f"  WARN: no gID in {path.relative_to(ROOT)}, skipping")
            continue

        if gid in gid_to_uid:
            print(f"  WARN: duplicate gID {gid} in {path.relative_to(ROOT)}")
            continue

        instance += 1
        uid = make_uid(DOMAIN_RS, KIND_VEHICLE, 0, instance)
        gid_to_uid[gid] = uid

        type_uid = type_id_to_uid.get(type_id) if type_id else None
        if type_uid is None and type_id:
            print(f"  WARN: unknown typeID {type_id!r} in {path.relative_to(ROOT)}")

        new_obj: dict = {"uid": uid}
        if type_uid is not None:
            new_obj["type_uid"] = type_uid
        for k, v in obj.items():
            if k not in ("gID", "typeID"):
                new_obj[k] = v

        if not dry_run:
            if backup:
                path.rename(path.with_suffix(".json.bak"))
            with open(path, "w", encoding="utf-8") as f:
                json.dump(new_obj, f, indent=2, ensure_ascii=False)
                f.write("\n")
        print(f"  Vehicle [{instance:4d}] {path.relative_to(ROOT)} → uid={uid:#x}")

    print(f"  → {len(gid_to_uid)} vehicles assigned UIDs")
    return gid_to_uid


# ── Trains ────────────────────────────────────────────────────────────────────

def migrate_trains(vehicle_gid_to_uid: dict[str, int], dry_run: bool, backup: bool) -> None:
    trains_dir = ROOT / "data" / "trains"
    files = sorted(trains_dir.rglob("*.json"))

    instance = 0
    for path in files:
        with open(path) as f:
            obj = json.load(f)

        gid = obj.get("gID")
        if gid is None:
            print(f"  WARN: no gID in {path.relative_to(ROOT)}, skipping")
            continue

        instance += 1
        uid = make_uid(DOMAIN_RS, KIND_TRAIN_CONSIST, 0, instance)

        vehicle_uids = []
        for vgid in obj.get("vehicles", []):
            vuid = vehicle_gid_to_uid.get(vgid)
            if vuid is None:
                print(f"  WARN: unknown vehicle gID {vgid!r} in {path.relative_to(ROOT)}")
            else:
                vehicle_uids.append(vuid)

        new_obj: dict = {"uid": uid}
        for k, v in obj.items():
            if k == "gID":
                continue
            elif k == "vehicles":
                new_obj["vehicle_uids"] = vehicle_uids
            else:
                new_obj[k] = v

        if not dry_run:
            if backup:
                path.rename(path.with_suffix(".json.bak"))
            with open(path, "w", encoding="utf-8") as f:
                json.dump(new_obj, f, indent=2, ensure_ascii=False)
                f.write("\n")
        print(f"  Train [{instance:4d}] {path.relative_to(ROOT)} → uid={uid:#x}")

    print(f"  → {instance} trains assigned UIDs")


# ── Topology ──────────────────────────────────────────────────────────────────

def collect_counter_ids(topology: dict) -> list[str]:
    """Collect all IT/IZ counter string IDs from topology port/leg definitions."""
    counters: list[str] = []
    for section in topology.get("track_sections", []):
        for side_key in ("sideA", "sideB"):
            side = section.get(side_key, {})
            for ckey in ("itID", "izID"):
                cid = side.get(ckey)
                if cid and cid not in counters:
                    counters.append(cid)
    for sw in topology.get("switches", []):
        for leg_key in ("trunk", "straight", "divergent"):
            leg = sw.get(leg_key, {})
            cid = leg.get("izID")
            if cid and cid not in counters:
                counters.append(cid)
    return counters


def migrate_scenario(scenario_dir: Path, station_code_to_instance: dict[str, int],
                     dry_run: bool, backup: bool) -> None:
    print(f"\n  Scenario: {scenario_dir.name}")

    # ── meta.json ─────────────────────────────────────────────────────────────
    meta_path = scenario_dir / "meta.json"
    with open(meta_path) as f:
        meta = json.load(f)

    station_sid = meta.get("station_sid", "")
    station_instance = station_code_to_instance.get(station_sid)
    if station_instance is None:
        print(f"  ERROR: unknown station_sid {station_sid!r} in meta.json")
        return

    station_uid = make_uid(DOMAIN_INFRA, KIND_STATION, station_instance, 1)

    # meta.json keeps station_sid as-is (used for display/control system lookup).
    # station_uid is derived from scope of each object's UID so no change needed.
    print(f"    meta.json: station_uid={station_uid:#x} (instance={station_instance}) [no change]")

    # ── topology.json ─────────────────────────────────────────────────────────
    topo_path = scenario_dir / "topology.json"
    with open(topo_path) as f:
        topo = json.load(f)

    # Collect all objects and counters
    all_gids: dict[str, tuple[int, int]] = {}  # gID → (kind, instance)
    counter_gids: dict[str, int] = {}  # counter_string_id → UID

    def assign_uid_for_category(items: list, kind: int) -> dict[str, int]:
        mapping: dict[str, int] = {}
        for inst, item in enumerate(items, start=1):
            gid = item.get("gID")
            if gid:
                mapping[gid] = make_uid(DOMAIN_INFRA, kind, station_instance, inst)
        return mapping

    topo_gid_to_uid: dict[str, int] = {}
    topo_gid_to_uid.update(assign_uid_for_category(topo.get("boundary_nodes", []), KIND_BOUNDARY_NODE))
    topo_gid_to_uid.update(assign_uid_for_category(topo.get("track_sections", []), KIND_TRACK_SECTION))
    topo_gid_to_uid.update(assign_uid_for_category(topo.get("switches", []), KIND_SWITCH))

    # Assign AXLE_COUNTER UIDs to IT/IZ counters
    counter_ids = collect_counter_ids(topo)
    for inst, cid in enumerate(counter_ids, start=1):
        counter_gids[cid] = make_uid(DOMAIN_INFRA, KIND_AXLE_COUNTER, station_instance, inst)

    def resolve(gid: str) -> int:
        uid = topo_gid_to_uid.get(gid)
        if uid is None:
            # May be a signal/derailer reference — will be resolved after objects.json
            uid = obj_gid_to_uid.get(gid, 0)
        return uid

    # ── objects.json (read first to build signal/derailer mappings) ───────────
    objects_path = scenario_dir / "objects.json"
    obj_gid_to_uid: dict[str, int] = {}
    objs_data: dict = {}

    if objects_path.exists():
        with open(objects_path) as f:
            objs_data = json.load(f)
        obj_gid_to_uid.update(assign_uid_for_category(objs_data.get("signals", []), KIND_SIGNAL))
        obj_gid_to_uid.update(assign_uid_for_category(objs_data.get("derailers", []), KIND_DERAILER))

    # Combined lookup
    all_uid_lookup = {**topo_gid_to_uid, **obj_gid_to_uid}

    def lookup_uid(gid: str) -> int:
        uid = all_uid_lookup.get(gid, 0)
        if uid == 0:
            print(f"    WARN: unresolved reference to gID {gid!r}")
        return uid

    def convert_port(port: dict) -> dict:
        new_port: dict = {}
        neighbor = port.get("neighborID", "")
        if neighbor:
            new_port["neighborUID"] = lookup_uid(neighbor)
        # IT counter (boundary-side) and IZ counter (switch-side) kept as separate fields
        it_id = port.get("itID")
        if it_id:
            new_port["itUID"] = counter_gids.get(it_id, 0)
        iz_id = port.get("izID")
        if iz_id:
            new_port["izUID"] = counter_gids.get(iz_id, 0)
        sigs = port.get("signals", [])
        new_port["signalUIDs"] = [lookup_uid(s) for s in sigs]
        return new_port

    def convert_switch_leg(leg: dict) -> dict:
        new_leg: dict = {}
        neighbor = leg.get("neighborID", "")
        if neighbor:
            new_leg["neighborUID"] = lookup_uid(neighbor)
        iz = leg.get("izID")
        if iz:
            new_leg["izUID"] = counter_gids.get(iz, 0)
        sigs = leg.get("signals", [])
        new_leg["signalUIDs"] = [lookup_uid(s) for s in sigs]
        return new_leg

    # Convert topology.json
    new_topo: dict = {}
    if "$comment" in topo:
        new_topo["$comment"] = topo["$comment"]

    new_boundary_nodes = []
    for item in topo.get("boundary_nodes", []):
        uid = topo_gid_to_uid.get(item["gID"], 0)
        new_item = {"uid": uid, "pID": item.get("pID", "")}
        if "description" in item:
            new_item["description"] = item["description"]
        new_boundary_nodes.append(new_item)
    new_topo["boundary_nodes"] = new_boundary_nodes

    new_sections = []
    for item in topo.get("track_sections", []):
        uid = topo_gid_to_uid.get(item["gID"], 0)
        new_item: dict = {
            "uid": uid,
            "pID": item.get("pID", ""),
            "sideA": convert_port(item.get("sideA", {})),
            "sideB": convert_port(item.get("sideB", {})),
            "lengthM": item.get("lengthM", 0.0),
            "electrified": item.get("electrified", False),
            "maxSpeedKmh": item.get("maxSpeedKmh", 0),
        }
        if "occupied" in item:
            new_item["occupied"] = item["occupied"]
        new_sections.append(new_item)
    new_topo["track_sections"] = new_sections

    new_switches = []
    for item in topo.get("switches", []):
        uid = topo_gid_to_uid.get(item["gID"], 0)
        new_item = {
            "uid": uid,
            "pID": item.get("pID", ""),
            "typeID": item.get("typeID", ""),
            "trunk":    convert_switch_leg(item.get("trunk", {})),
            "straight": convert_switch_leg(item.get("straight", {})),
            "divergent": convert_switch_leg(item.get("divergent", {})),
        }
        for extra in ("lengthM", "maxSpeedStraightKmh", "maxSpeedDivergentKmh"):
            if extra in item:
                new_item[extra] = item[extra]
        new_switches.append(new_item)
    new_topo["switches"] = new_switches

    if not dry_run:
        if backup:
            topo_path.rename(topo_path.with_suffix(".json.bak"))
        with open(topo_path, "w", encoding="utf-8") as f:
            json.dump(new_topo, f, indent=4, ensure_ascii=False)
            f.write("\n")
    print(f"    topology.json: {len(new_boundary_nodes)} BND, {len(new_sections)} TS, {len(new_switches)} SW")

    # Convert objects.json
    if objects_path.exists():
        new_objs: dict = {}
        if "$comment" in objs_data:
            new_objs["$comment"] = objs_data["$comment"]

        new_signals = []
        for item in objs_data.get("signals", []):
            uid = obj_gid_to_uid.get(item["gID"], 0)
            governs = lookup_uid(item.get("governs_track_section", ""))
            new_item = {
                "uid": uid,
                "pID": item.get("pID", ""),
                "typeID": item.get("typeID", ""),
                "type": item.get("type", ""),
                "governs_section": governs,
                "initial_aspect": item.get("initial_aspect", "STOP"),
            }
            new_signals.append(new_item)
        new_objs["signals"] = new_signals

        new_derailers = []
        for item in objs_data.get("derailers", []):
            uid = obj_gid_to_uid.get(item["gID"], 0)
            guards = lookup_uid(item.get("guards_track_section", ""))
            new_item = {
                "uid": uid,
                "pID": item.get("pID", ""),
                "typeID": item.get("typeID", ""),
                "guards_section": guards,
            }
            new_derailers.append(new_item)
        new_objs["derailers"] = new_derailers

        if not dry_run:
            if backup:
                objects_path.rename(objects_path.with_suffix(".json.bak"))
            with open(objects_path, "w", encoding="utf-8") as f:
                json.dump(new_objs, f, indent=4, ensure_ascii=False)
                f.write("\n")
        print(f"    objects.json: {len(new_signals)} signals, {len(new_derailers)} derailers")


# ── Main ──────────────────────────────────────────────────────────────────────

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--backup", action="store_true", help="Backup originals as .bak")
    parser.add_argument("--dry-run", action="store_true", help="Do not write files")
    args = parser.parse_args()

    if args.dry_run:
        print("DRY RUN — no files will be written")

    stations = load_stations()
    print(f"Loaded {len(stations)} station(s): {stations}")

    print("\n=== Vehicle types ===")
    type_id_to_uid = migrate_vehicle_types(args.dry_run, args.backup)

    print("\n=== Vehicles ===")
    vehicle_gid_to_uid = migrate_vehicles(type_id_to_uid, args.dry_run, args.backup)

    print("\n=== Trains ===")
    migrate_trains(vehicle_gid_to_uid, args.dry_run, args.backup)

    print("\n=== Scenarios ===")
    scenarios_dir = ROOT / "scenarios"
    for scenario_dir in sorted(scenarios_dir.iterdir()):
        if not scenario_dir.is_dir():
            continue
        meta_path = scenario_dir / "meta.json"
        if not meta_path.exists():
            continue
        migrate_scenario(scenario_dir, stations, args.dry_run, args.backup)

    print("\nDone.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
