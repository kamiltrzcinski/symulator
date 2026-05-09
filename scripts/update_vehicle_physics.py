#!/usr/bin/env python3
"""
Update all vehicle_type JSON files with:
  - tractionForceKN  (max starting traction force in kN)
  - davisA           (constant resistance term, N)
  - davisB           (speed-linear term, N per km/h)
  - davisC           (aerodynamic term, N per (km/h)^2)

Davis formula (per-tonne, matching engine::core::DavisCoefficients):
  F_resistance [N] = (davisA + davisB * v[km/h] + davisC * v[km/h]^2) * effective_mass_t

Units:
  davisA  [N/t]
  davisB  [N / (t · km/h)]
  davisC  [N / (t · (km/h)^2)]

Davis constants derived from standard PKP/UIC specific resistance:
  a_pt [N/t] = a_spec * 1000 * g   (where a_spec is dimensionless, g=9.81 m/s²)

where a_spec, b_spec, c_spec are the dimensionless Strahl/UIC specific resistance factors.

Traction force is set from:
  1. Known exact values (from Wikipedia / manufacturer specs)
  2. Power-based formula: F = powerKW * 1000 / v_crossover_ms
  3. Wagons and trailers: null (no traction)

Sources:
  - ET22 (201E): 411 kN, Wikipedia https://pl.wikipedia.org/wiki/ET22
  - EU07 (4E/303E): 280 kN, Wikipedia https://pl.wikipedia.org/wiki/EU07
  - EP09 (104E): 195 kN, Wikipedia https://pl.wikipedia.org/wiki/EP09
  - EN57 (5B/6B): ~80 kN (from mass 126.5t and a=0.5 m/s^2 starting acceleration)
  - SA134: ~95 kN (from power/typical DMU crossover speed)
"""

import json
import os
import math

# vehicle_types/ now uses subdirectories: locomotive/{electric,diesel,steam},
# emu_unit/motor, dmu_unit/motor, freight_wagon/hopper, service_wagon
DATA_DIR = os.path.join(os.path.dirname(__file__), "..", "data", "vehicle_types")
G = 9.81  # m/s^2

# ──────────────────────────────────────────────────────────────────────────────
# 1. KNOWN EXACT TRACTION FORCES (kN) — from manufacturer specs / Wikipedia
# ──────────────────────────────────────────────────────────────────────────────
KNOWN_TRACTION_KN: dict[str, float] = {
    # Classic PKP electric locomotives
    "ET22":    411.0,   # Pafawag 201E, Co'Co', 120t, freight
    "ET41":    560.0,   # HCP 203E, Bo'Bo'+Bo'Bo', 167t, 2×280
    "ET42":    560.0,   # similar to ET41 double-section
    "EU07":    280.0,   # Pafawag 4E / HCP 303E, Bo'Bo', 80t
    "EP07":    280.0,   # re-geared EU07 (same mechanics)
    "EP08":    270.0,   # Pafawag 102E
    "EU06":    280.0,   # English Electric AEI-E, same family
    "EU22":    250.0,   # Pafawag 202E derivative
    "EP09":    195.0,   # Pafawag 104E, Bo'Bo', 83.5t, 160 km/h
    # Modern electric locomotives
    "E6ACT":   360.0,   # Newag Dragon I, 119t, Bo'Bo'
    "E6ACTa":  360.0,   # Newag Dragon I variant
    "E6ACTad": 360.0,
    "E6ACTab": 360.0,
    "E6ACTd":  360.0,
    "E6MST":   360.0,   # Newag 311D
    "ES64U4":  300.0,   # Siemens Taurus ÖBB/PKP variant
    "ES64F4":  300.0,   # Siemens EuroSprinter
    # Classic PKP diesel locomotives
    "SM42":    200.0,   # Fablok 6D, Bo'Bo', 70t, 588 kW
    "SP42":    200.0,   # same as SM42
    "SM48":    230.0,   # Fablok 9D, heavier shunter
    "SM31":    150.0,   # light shunter
    "SM03":     60.0,   # small diesel shunter
    "SU46":    260.0,   # FABLOK 10D, passenger diesel
    "ST44":    300.0,   # Soviet M62 variant, freight diesel
    "ST45":    260.0,
    "M62":     290.0,   # base M62
    # Classic EMU units
    "EN57":     80.0,   # Pafawag 5B/6B, 3-car, 580 kW, 126.5t
    "EN71":    100.0,   # Pafawag 5Bg/6Bg, 3-car, enhanced
    # DMU units
    "SA134":    95.0,   # 700 kW, 2-car, 78.5t
    "SA136":    95.0,   # similar to SA134
    "SA106":    60.0,   # 395 kW, single-car
    "SA108":    70.0,
    "SA109":    70.0,
    "SA132":    85.0,
    "SA133":    90.0,
    "SA135":    95.0,
}

# Normalise keys: strip to typeName pattern (uppercase, strip trailing alpha suffixes
# loosely matching) — but we also do direct exact match and case-insensitive match.

# ──────────────────────────────────────────────────────────────────────────────
# 2. FORMULA FALLBACK — crossover speed (m/s) by vehicle category
# ──────────────────────────────────────────────────────────────────────────────
# F_traction [N] = powerKW * 1000 / v_crossover_ms
# Represents speed at which constant-power regime begins (below this speed,
# traction force is constant; above it, it decreases with speed).
#
# Derived from: EU07 2000 kW / 280 kN = 7.14 m/s (25.7 km/h)
#               ET22 3000 kW / 411 kN = 7.30 m/s (26.3 km/h)
#               EP09 2920 kW / 195 kN = 14.97 m/s (53.9 km/h)

def compute_traction_kn(type_name: str, vehicle_type: str, vehicle_subtype: str,
                        power_kw: float | None, vmax_kmh: float) -> float | None:
    """Return traction force in kN or None for non-traction vehicles."""

    # 1. Check exact known values first (case-insensitive)
    key = type_name.upper()
    for k, v in KNOWN_TRACTION_KN.items():
        if k.upper() == key:
            return v

    # 2. For wagons / trailers: no traction
    non_traction_types = {"PASSENGER_WAGON", "FREIGHT_WAGON", "SERVICE_WAGON"}
    non_traction_subtypes = {"TRAILER", "CONTROL"}
    if vehicle_type in non_traction_types:
        return None
    if vehicle_subtype in non_traction_subtypes:
        return None

    # 3. Compute from power
    if power_kw is None or power_kw <= 0:
        return None

    if vehicle_type == "LOCOMOTIVE":
        if vehicle_subtype == "ELECTRIC":
            if vmax_kmh and vmax_kmh >= 140:
                # High-speed passenger electric: high crossover speed
                v_c = 15.0  # 54 km/h
            else:
                # Freight/universal electric: low crossover speed
                v_c = 7.3   # 26 km/h
        elif vehicle_subtype == "DIESEL":
            if vmax_kmh and vmax_kmh <= 100:
                # Shunter/slow diesel: very low crossover
                v_c = 3.0   # 10.8 km/h
            else:
                # Mainline diesel
                v_c = 7.0   # 25 km/h
        else:
            v_c = 7.5
    elif vehicle_type in ("EMU_UNIT", "DMU_UNIT"):
        if vehicle_subtype == "MOTOR":
            v_c = 7.0  # 25 km/h typical for EMU/DMU starting
        else:
            return None
    else:
        return None

    f_kn = power_kw / v_c  # kN (kW / (m/s) = kN)
    return round(f_kn, 1)


# ──────────────────────────────────────────────────────────────────────────────
# 3. DAVIS SPECIFIC RESISTANCE FACTORS
# ──────────────────────────────────────────────────────────────────────────────
# F_res [N] = (a + b*v + c*v²) * mass_t * 1000 * g
# where v is in km/h, g = 9.81 m/s²
# These are dimensionless specific resistance coefficients.
#
# Sources: PKP/UIC standard resistance formulae (Strahl/ORE B12):
#  - Passenger wagons: from ORE B12 Rp. 15
#  - Freight wagons:   lighter specific resistance per UIC 774-1
#  - Locomotives:      higher constant due to gearbox/bogie complexity

DAVIS_SPEC: dict[tuple, tuple] = {
    # (vehicleType, vehicleSubtype): (a, b, c)
    # a [1], b [h/km], c [(h/km)^2]
    ("LOCOMOTIVE",   "ELECTRIC"): (4.0e-3, 2.0e-5, 1.8e-7),  # standard ≤130 km/h
    ("LOCOMOTIVE",   "DIESEL"):   (4.5e-3, 2.2e-5, 2.0e-7),
    ("EMU_UNIT",     "MOTOR"):    (3.5e-3, 1.8e-5, 1.5e-7),
    ("EMU_UNIT",     "TRAILER"):  (3.0e-3, 1.5e-5, 1.4e-7),
    ("EMU_UNIT",     "CONTROL"):  (3.0e-3, 1.5e-5, 1.4e-7),
    ("DMU_UNIT",     "MOTOR"):    (4.0e-3, 2.0e-5, 1.8e-7),
    ("DMU_UNIT",     "TRAILER"):  (3.0e-3, 1.5e-5, 1.4e-7),
    ("DMU_UNIT",     "CONTROL"):  (3.0e-3, 1.5e-5, 1.4e-7),
    ("PASSENGER_WAGON", None):    (3.0e-3, 1.5e-5, 1.4e-7),
    ("FREIGHT_WAGON", None):      (1.5e-3, 8.0e-6, 8.0e-8),
    ("SERVICE_WAGON", None):      (2.5e-3, 1.3e-5, 1.2e-7),
}

# High-speed electric loco gets higher aerodynamic C
DAVIS_SPEC_HIGHSPEED_ELECTRIC = (3.5e-3, 1.8e-5, 2.5e-7)


def compute_davis(vehicle_type: str, vehicle_subtype: str,
                  mass_gross_t: float, vmax_kmh: float) -> tuple[float, float, float]:
    """Return per-tonne Davis constants (davisA, davisB, davisC).

    Units:
        davisA  [N/t]
        davisB  [N / (t · km/h)]
        davisC  [N / (t · (km/h)^2)]

    Formula at runtime:
        F_res [N] = (davisA + davisB * v_kmh + davisC * v_kmh^2) * effective_mass_t
    """

    # Select specific resistance factors
    if (vehicle_type == "LOCOMOTIVE" and vehicle_subtype == "ELECTRIC"
            and vmax_kmh and vmax_kmh >= 140):
        a, b, c = DAVIS_SPEC_HIGHSPEED_ELECTRIC
    else:
        key = (vehicle_type, vehicle_subtype)
        if key not in DAVIS_SPEC:
            # Fall back to (type, None) for wagon-like types
            key = (vehicle_type, None)
        if key not in DAVIS_SPEC:
            # Ultimate fallback: generic loco
            a, b, c = (4.0e-3, 2.0e-5, 1.8e-7)
        else:
            a, b, c = DAVIS_SPEC[key]

    # Convert to per-tonne coefficients: a_pt [N/t] = a_spec * 1000 * G
    # (a_spec is dimensionless, 1000 converts t→kg, G = 9.81 m/s²)
    K = 1000.0 * G          # 9810 (N/t factor)
    A_pt = a * K
    B_pt = b * K
    C_pt = c * K
    return round(A_pt, 3), round(B_pt, 5), round(C_pt, 7)


# ──────────────────────────────────────────────────────────────────────────────
# 4. MAIN: process all files
# ──────────────────────────────────────────────────────────────────────────────

def process_file(path: str) -> tuple[bool, str]:
    """Return (was_changed, message)."""
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)

    type_name       = data.get("typeName", "")
    vehicle_type    = data.get("vehicleType", "")
    vehicle_subtype = data.get("vehicleSubtype", "")
    power_kw        = data.get("powerKW")
    vmax_kmh        = data.get("maxSpeedKmh", 0) or 0
    mass_gross_t    = data.get("massGrossT") or data.get("massEmptyT") or 0

    changes = []

    # ── traction force ──────────────────────────────────────────────────────
    current_traction = data.get("tractionForceKN")
    # Fix obviously wrong value: tractionForceKN on a wagon (should be null)
    non_traction_types = {"PASSENGER_WAGON", "FREIGHT_WAGON", "SERVICE_WAGON"}
    non_traction_subtypes = {"TRAILER", "CONTROL"}
    if (current_traction is not None and
            (vehicle_type in non_traction_types or
             vehicle_subtype in non_traction_subtypes)):
        data["tractionForceKN"] = None
        changes.append(f"tractionForceKN: {current_traction} → null (non-traction vehicle)")
        current_traction = None

    if current_traction is None:
        new_traction = compute_traction_kn(
            type_name, vehicle_type, vehicle_subtype, power_kw, vmax_kmh)
        if new_traction is not None:
            data["tractionForceKN"] = new_traction
            changes.append(f"tractionForceKN = {new_traction}")

    # ── Davis constants ─────────────────────────────────────────────────────
    # Always regenerate — ensures per-tonne units after any format change.
    if mass_gross_t > 0:
        da, db, dc = compute_davis(
            vehicle_type, vehicle_subtype, mass_gross_t, vmax_kmh)
        if (data.get("davisA") != da or data.get("davisB") != db
                or data.get("davisC") != dc):
            data["davisA"] = da
            data["davisB"] = db
            data["davisC"] = dc
            changes.append(f"Davis({da}, {db}, {dc})")

    if not changes:
        return False, "no changes"

    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2, ensure_ascii=False)
        f.write("\n")

    return True, "; ".join(changes)


def main():
    # Recursively collect all JSON files from the subdirectory tree
    import glob as _glob
    files = sorted(_glob.glob(os.path.join(DATA_DIR, "**", "*.json"), recursive=True))
    total = len(files)
    changed = 0
    errors = []

    for path in files:
        fname = os.path.relpath(path, DATA_DIR)
        try:
            was_changed, msg = process_file(path)
            if was_changed:
                changed += 1
                print(f"  ✓ {fname}: {msg}")
        except Exception as e:
            errors.append((fname, str(e)))
            print(f"  ✗ {fname}: ERROR — {e}")

    print(f"\nDone: {changed}/{total} files updated, {len(errors)} errors.")
    if errors:
        for fname, err in errors:
            print(f"  ERROR {fname}: {err}")


if __name__ == "__main__":
    main()
