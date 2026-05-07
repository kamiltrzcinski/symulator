#!/usr/bin/env python3
"""
Generate structured datasets for Polish rolling stock: Diesel locomotives, EMU, DMU (SZT)
Creates individual JSON files for each vehicle type without index files.
"""

import os
import json
from pathlib import Path

# Define base path
BASE_DIR = Path(__file__).parent.parent
DATA_DIR = BASE_DIR / "data" / "vehicle_types"

# Ensure directory exists
DATA_DIR.mkdir(parents=True, exist_ok=True)

# Vehicle type lists
diesel = [
    "060Da", "111Db", "15D", "15D/A", "16D", "16D/A", "301Db", "311D", "401Da", "409Da", "6De",
    "6Dg", "6Dh", "6Dm", "753", "754", "BR231", "BR232", "BR233", "BR285", "Class66", "Ls1000",
    "Ls1200", "Ls150", "Ls180", "Ls800", "M62", "M62BF", "M62Ko", "M62M", "M62Y", "MaK DE6400",
    "S200", "SM03", "SM04", "SM30", "SM31", "SM32", "SM40", "SM41", "SM42", "SM48",
    "SP32", "SP42", "SP45", "SP47", "SP49", "ST40", "ST43", "ST44", "ST45", "ST46", "ST48",
    "SU160", "SU42", "SU4210", "SU45", "SU46", "T448p", "TEM2"
]

emu = [
    "14WE", "19WE", "21WE", "21WEa", "22WE", "22WEa", "22WEc", "22WEd", "22WEe",
    "27WE", "27WEb", "31WE", "31WEba", "34WE", "34WEa", "35WE", "36WE", "36WEa",
    "36WEd", "36WEdb", "37WE", "37WEa", "45WE", "48WE", "48WEc", "ED160", "ED161",
    "ED250", "ED59", "ED72", "ED72A", "ED72Ac", "ED73", "ED74", "ED78", "EN57", "EN57AKD",
    "EN57AKŁ", "EN57AKM", "EN57AKS", "EN57AL", "EN57ALc", "EN57ALd", "EN57AP",
    "EN57FPS", "EN57KM", "EN61", "EN62", "EN62A", "EN63", "EN63A", "EN64", "EN71",
    "EN75", "EN76", "EN76A", "EN77", "EN78", "EN79", "EN81", "EN90", "EN96", "EN96A", "EN98",
    "EN99", "ER160", "ER75", "EW55", "EW58", "EW60", "L-4268", "LM-4268"
]

dmu = [
    "222M", "36WEh", "36WEhd", "810", "840", "DSB MA", "EN63H", "MITOR", "MRD", "SA101",
    "SA102", "SA103", "SA104", "SA105", "SA106", "SA107", "SA108", "SA109", "SA110",
    "SA123", "SA131", "SA132", "SA133", "SA134", "SA135", "SA136", "SA137", "SA138",
    "SA139", "SA140", "SN61", "SN81", "SN82", "SN83", "SN84", "VT627", "VT628"
]

def create_template(name, vehicle_type):
    """Create a vehicle type template JSON structure."""
    return {
        "typeID": f"VT-{vehicle_type.upper()}-{name.replace('/', '_')}-0000001",
        "typeName": name,
        "vehicleType": vehicle_type.upper(),
        "vehicleSubtype": None,
        "lengthM": None,
        "axleCount": None,
        "massEmptyT": None,
        "massGrossT": None,
        "maxSpeedKmh": None,
        "brakingLambdaPct": None,
        "powerKW": None,
        "tractionForceKN": None,
        "family": None,
        "sourceReliability": "estimated"
    }

def write_vehicle_group(vehicles, vehicle_type):
    """Write vehicle JSON files for a specific type (DIESEL, EMU, DMU)."""
    created_files = []
    
    for vehicle_name in vehicles:
        data = create_template(vehicle_name, vehicle_type)
        file_name = vehicle_name.replace('/', '_').lower()
        file_path = DATA_DIR / f"{file_name}.json"
        
        try:
            with open(file_path, "w", encoding="utf-8") as f:
                json.dump(data, f, ensure_ascii=False, indent=2)
            created_files.append(str(file_path))
            print(f"✓ Created: {file_name}.json ({vehicle_type})")
        except IOError as e:
            print(f"✗ Error writing {file_name}.json: {e}")
    
    return created_files

def main():
    """Main execution function."""
    print(f"Vehicle Type Generator\n{'='*50}\n")
    print(f"Output directory: {DATA_DIR}\n")
    
    all_files = []
    
    # Generate diesel locomotives
    print("Generating Diesel Locomotives...")
    diesel_files = write_vehicle_group(diesel, "DIESEL")
    all_files.extend(diesel_files)
    print(f"  Total: {len(diesel_files)} files\n")
    
    # Generate EMU (Electric Multiple Units)
    print("Generating EMU (Electric Multiple Units)...")
    emu_files = write_vehicle_group(emu, "EMU")
    all_files.extend(emu_files)
    print(f"  Total: {len(emu_files)} files\n")
    
    # Generate DMU (Diesel Multiple Units)
    print("Generating DMU (Diesel Multiple Units)...")
    dmu_files = write_vehicle_group(dmu, "DMU")
    all_files.extend(dmu_files)
    print(f"  Total: {len(dmu_files)} files\n")
    
    print(f"{'='*50}")
    print(f"Generation Complete!\n")
    print(f"Total files created: {len(all_files)}")
    print(f"  - Diesel locomotives: {len(diesel_files)}")
    print(f"  - EMU: {len(emu_files)}")
    print(f"  - DMU: {len(dmu_files)}")

if __name__ == "__main__":
    main()
