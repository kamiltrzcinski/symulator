#!/usr/bin/env python3
"""Validate all UID values in JSON data and topology files.

Checks performed:
  - All uid/type_uid/station_uid values are valid uint64 <= 2^53 - 1
  - DOMAIN, KIND, SCOPE, INSTANCE fields decode to known valid values
  - INSTANCE != 0
  - SCOPE for INFRASTRUCTURE kinds must be a known station instance
  - No duplicate UIDs within a single file
  - vehicle_uids in trains reference existing vehicle UIDs (cross-file check)

Exit code: 0 on success, 1 on any validation failure.
"""

import json
import os
import sys
from pathlib import Path

ROOT = Path(__file__).parent.parent

MAX_SAFE_JSON_INTEGER = (1 << 53) - 1

# Known station instances from data/stations.json
KNOWN_STATIONS: dict[int, str] = {}

# Valid UIDDomain values
DOMAINS = {0x01: "ROLLING_STOCK", 0x02: "INFRASTRUCTURE", 0x03: "OPERATIONS"}

# Valid UIDKind values (globally unique)
KINDS = {
    0x01: "VEHICLE_TYPE",
    0x02: "VEHICLE",
    0x03: "TRAIN_CONSIST",
    0x04: "CARRIER",
    0x11: "STATION",
    0x12: "DISPATCH_AREA",
    0x13: "TRACK_SECTION",
    0x14: "SWITCH",
    0x15: "SIGNAL",
    0x16: "DERAILER",
    0x17: "BLOCK_SECTION",
    0x18: "BOUNDARY_NODE",
    0x19: "LEVEL_CROSSING",
    0x1A: "AXLE_COUNTER",
    0x1B: "INTERLOCKING",
    0x1C: "POWER_SUPPLY",
    0x21: "ROUTE",
    0x22: "ALARM",
    0x23: "DISPATCH_EXCHANGE",
}

INFRA_KINDS = {k for k, v in KINDS.items() if 0x10 < k < 0x20}
ROLLING_STOCK_KINDS = {k for k, v in KINDS.items() if k < 0x10}
OPS_KINDS = {k for k, v in KINDS.items() if k >= 0x20}


def uid_domain(value: int) -> int:
    return (value >> 40) & 0xFF


def uid_kind(value: int) -> int:
    return (value >> 32) & 0xFF


def uid_scope(value: int) -> int:
    return (value >> 16) & 0xFFFF


def uid_instance(value: int) -> int:
    return value & 0xFFFF


errors: list[str] = []


def err(path: str, msg: str) -> None:
    errors.append(f"{path}: {msg}")
    print(f"  ERROR  {path}: {msg}", file=sys.stderr)


def validate_uid(value, field_name: str, file_path: str) -> bool:
    """Return True if the UID value is valid."""
    if not isinstance(value, int):
        err(file_path, f"{field_name} is not an integer: {value!r}")
        return False
    if value < 0:
        err(file_path, f"{field_name} is negative: {value}")
        return False
    if value > MAX_SAFE_JSON_INTEGER:
        err(file_path, f"{field_name} exceeds 2^53-1: {value}")
        return False

    domain = uid_domain(value)
    kind = uid_kind(value)
    scope = uid_scope(value)
    instance = uid_instance(value)

    if domain not in DOMAINS:
        err(file_path, f"{field_name}={value:#x}: unknown DOMAIN {domain:#x}")
        return False

    if kind not in KINDS:
        err(file_path, f"{field_name}={value:#x}: unknown KIND {kind:#x}")
        return False

    if instance == 0:
        err(file_path, f"{field_name}={value:#x}: INSTANCE is 0 (reserved/invalid)")
        return False

    # SCOPE validation for INFRASTRUCTURE
    if kind in INFRA_KINDS and scope != 0:
        if KNOWN_STATIONS and scope not in KNOWN_STATIONS:
            err(
                file_path,
                f"{field_name}={value:#x}: SCOPE {scope} is not a known station instance "
                f"(known: {sorted(KNOWN_STATIONS)})",
            )
            return False

    return True


def load_stations() -> None:
    stations_file = ROOT / "scenarios" / "stations.json"
    if not stations_file.exists():
        print(f"WARNING: {stations_file} not found; skipping station-scope validation")
        return
    with open(stations_file) as f:
        data = json.load(f)
    for entry in data:
        KNOWN_STATIONS[entry["instance"]] = entry["code"]
    print(f"  Loaded {len(KNOWN_STATIONS)} station(s) from stations.json")


def validate_vehicle_types() -> None:
    types_dir = ROOT / "packages" / "vehicle-types"
    if not types_dir.exists():
        return

    seen_uids: dict[int, str] = {}
    count = 0
    for path in sorted(types_dir.rglob("*.json")):
        with open(path) as f:
            try:
                obj = json.load(f)
            except json.JSONDecodeError as e:
                err(str(path), f"JSON parse error: {e}")
                continue

        if "uid" not in obj:
            err(str(path), "missing 'uid' field")
            continue

        uid_val = obj["uid"]
        rel = str(path.relative_to(ROOT))
        validate_uid(uid_val, "uid", rel)

        kind = uid_kind(uid_val) if isinstance(uid_val, int) else None
        if kind is not None and kind != 0x01:
            err(rel, f"uid has wrong KIND {kind:#x} for vehicle_type (expected VEHICLE_TYPE=0x01)")

        if isinstance(uid_val, int) and uid_val in seen_uids:
            err(rel, f"duplicate uid {uid_val:#x} (also in {seen_uids[uid_val]})")
        elif isinstance(uid_val, int):
            seen_uids[uid_val] = rel
        count += 1

    print(f"  Checked {count} vehicle_type file(s)")


def validate_vehicles() -> dict[int, str]:
    """Returns mapping uid → file for cross-reference checking."""
    vehicles_dir = ROOT / "packages" / "vehicles"
    if not vehicles_dir.exists():
        return {}

    seen_uids: dict[int, str] = {}
    count = 0
    for path in sorted(vehicles_dir.rglob("vehicle.json")):
        with open(path) as f:
            try:
                obj = json.load(f)
            except json.JSONDecodeError as e:
                err(str(path), f"JSON parse error: {e}")
                continue

        rel = str(path.relative_to(ROOT))

        for field in ("uid", "type_uid"):
            if field in obj:
                validate_uid(obj[field], field, rel)

        if "uid" not in obj:
            err(rel, "missing 'uid' field")
            continue

        uid_val = obj["uid"]
        kind = uid_kind(uid_val) if isinstance(uid_val, int) else None
        if kind is not None and kind != 0x02:
            err(rel, f"uid has wrong KIND {kind:#x} for vehicle (expected VEHICLE=0x02)")

        if isinstance(uid_val, int) and uid_val in seen_uids:
            err(rel, f"duplicate uid {uid_val:#x} (also in {seen_uids[uid_val]})")
        elif isinstance(uid_val, int):
            seen_uids[uid_val] = rel
        count += 1

    print(f"  Checked {count} vehicle file(s)")
    return seen_uids


def validate_trains(vehicle_uids: dict[int, str]) -> None:
    train_dirs = [ROOT / "packages" / "trains"]

    seen_uids: dict[int, str] = {}
    count = 0
    for path in sorted(
        p for d in train_dirs if d.exists() for p in d.rglob("*.json")
    ):
        with open(path) as f:
            try:
                obj = json.load(f)
            except json.JSONDecodeError as e:
                err(str(path), f"JSON parse error: {e}")
                continue

        rel = str(path.relative_to(ROOT))
        if "uid" not in obj:
            err(rel, "missing 'uid' field")
            continue

        uid_val = obj["uid"]
        validate_uid(uid_val, "uid", rel)

        kind = uid_kind(uid_val) if isinstance(uid_val, int) else None
        if kind is not None and kind != 0x03:
            err(rel, f"uid has wrong KIND {kind:#x} for train (expected TRAIN_CONSIST=0x03)")

        if isinstance(uid_val, int) and uid_val in seen_uids:
            err(rel, f"duplicate uid {uid_val:#x} (also in {seen_uids[uid_val]})")
        elif isinstance(uid_val, int):
            seen_uids[uid_val] = rel

        if vehicle_uids and "vehicle_uids" in obj:
            for ref in obj["vehicle_uids"]:
                if isinstance(ref, int) and ref not in vehicle_uids:
                    err(rel, f"vehicle_uids references unknown vehicle uid {ref:#x}")
        count += 1

    print(f"  Checked {count} train file(s)")


def validate_carriers() -> None:
    carriers_file = ROOT / "packages" / "carriers" / "carriers.json"
    if not carriers_file.exists():
        return

    with open(carriers_file) as f:
        try:
            obj = json.load(f)
        except json.JSONDecodeError as e:
            err(str(carriers_file), f"JSON parse error: {e}")
            return

    rel = str(carriers_file.relative_to(ROOT))
    carriers = obj.get("carriers", [])
    seen_uids: dict[int, int] = {}
    for i, carrier in enumerate(carriers):
        if not isinstance(carrier, dict):
            err(rel, f"carriers[{i}] is not an object")
            continue
        uid_val = carrier.get("id")
        if uid_val is None:
            err(rel, f"carriers[{i}] missing 'id'")
            continue
        validate_uid(uid_val, f"carriers[{i}].id", rel)

        kind = uid_kind(uid_val) if isinstance(uid_val, int) else None
        if kind is not None and kind != 0x04:
            err(rel, f"carriers[{i}].id has wrong KIND {kind:#x} (expected CARRIER=0x04)")

        if isinstance(uid_val, int) and uid_val in seen_uids:
            err(rel, f"duplicate carrier id {uid_val:#x} (also at index {seen_uids[uid_val]})")
        elif isinstance(uid_val, int):
            seen_uids[uid_val] = i

    print(f"  Checked {len(carriers)} carrier(s)")


def _validate_topology_obj(obj: dict, rel: str, seen_uids: dict[int, str]) -> None:
    if "gID" in obj:
        err(rel, f"found old 'gID' field (must be 'uid' with numeric value): {obj.get('gID')}")
    if "sID" in obj:
        err(rel, "found old 'sID' field (must be removed; station encoded in SCOPE)")

    if "uid" not in obj:
        return

    uid_val = obj["uid"]
    validate_uid(uid_val, "uid", rel)

    if isinstance(uid_val, int) and uid_val in seen_uids:
        err(rel, f"duplicate uid {uid_val:#x}")
    elif isinstance(uid_val, int):
        seen_uids[uid_val] = str(uid_val)

    # Validate UID fields nested inside port sub-objects
    # (sideA/sideB for track sections, trunk/straight/divergent for switches)
    for port_key in ("sideA", "sideB", "trunk", "straight", "divergent"):
        port = obj.get(port_key)
        if not isinstance(port, dict):
            continue
        for key in ("neighborUID", "itUID", "izUID", "counterUID"):
            if key in port:
                validate_uid(port[key], f"{port_key}.{key}", rel)
        if isinstance(port.get("signalUIDs"), list):
            for j, ref in enumerate(port["signalUIDs"]):
                validate_uid(ref, f"{port_key}.signalUIDs[{j}]", rel)

    # Also handle any top-level UID fields (flat legacy format)
    for key in ("neighborUID", "counterUID", "itUID", "izUID"):
        if key in obj:
            validate_uid(obj[key], key, rel)
    for key in ("signalUIDs", "szlak_section_uids", "section_uids", "switch_uids", "derailer_uids"):
        if key in obj and isinstance(obj[key], list):
            for j, ref in enumerate(obj[key]):
                validate_uid(ref, f"{key}[{j}]", rel)


def validate_topology(topology_path: Path) -> None:
    with open(topology_path) as f:
        try:
            data = json.load(f)
        except json.JSONDecodeError as e:
            err(str(topology_path), f"JSON parse error: {e}")
            return

    rel = str(topology_path.relative_to(ROOT))
    seen_uids: dict[int, str] = {}

    if isinstance(data, list):
        objects: list = data
    elif isinstance(data, dict):
        objects = []
        for key in ("boundary_nodes", "track_sections", "switches"):
            section = data.get(key, [])
            if isinstance(section, list):
                objects.extend(section)
    else:
        err(rel, "topology root must be a list or object")
        return

    for obj in objects:
        if isinstance(obj, dict):
            _validate_topology_obj(obj, rel, seen_uids)

    print(f"  Checked {len(seen_uids)} topology UID(s) in {topology_path.name}")


def validate_scenarios() -> None:
    scenarios_dir = ROOT / "scenarios"
    if not scenarios_dir.exists():
        return
    for topology_path in sorted(scenarios_dir.rglob("topology.json")):
        validate_topology(topology_path)


def main() -> int:
    print("=== UID Registry Validation ===")
    load_stations()

    print("\n-- Vehicle types --")
    validate_vehicle_types()

    print("\n-- Vehicles --")
    vehicle_uids = validate_vehicles()

    print("\n-- Trains --")
    validate_trains(vehicle_uids)

    print("\n-- Carriers --")
    validate_carriers()

    print("\n-- Scenario topology --")
    validate_scenarios()

    print()
    if errors:
        print(f"FAILED: {len(errors)} error(s) found.", file=sys.stderr)
        return 1
    print("OK: all UID values valid.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
