#pragma once

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "engine/core/types.hpp"

// ── Fleet data model ──────────────────────────────────────────────────────────
// Three-level hierarchy: VehicleType → Vehicle (instance) → TrainConsist.
// Property resolution rule (doc 10):
//   effective(field) = instance.field  if present
//                      type.field      otherwise

namespace engine::core
{

// ── Davis running-resistance coefficients ─────────────────────────────────────
// F_resistance [N] = (davis_a + davis_b * v_kmh + davis_c * v_kmh²) * mass_t
// Units: davis_a [N/t], davis_b [N/(t·km/h)], davis_c [N/(t·(km/h)²)]
//
// JSON fields: "davisA", "davisB", "davisC".
// If absent from the vehicle-type JSON, category defaults are applied:
//   LOCOMOTIVE (ELECTRIC, ≤130 km/h): a=39.24,  b=0.1962,  c=0.0017658
//   LOCOMOTIVE (ELECTRIC, >130 km/h): a=34.335, b=0.17658, c=0.0024525
//   LOCOMOTIVE (DIESEL)             : a=44.145, b=0.21582, c=0.0019620
//   EMU_UNIT  (MOTOR)               : a=34.335, b=0.17658, c=0.0014715
//   EMU_UNIT  (TRAILER/CONTROL)     : a=29.43,  b=0.14715, c=0.0013734
//   DMU_UNIT  (MOTOR)               : a=39.24,  b=0.1962,  c=0.0017658
//   DMU_UNIT  (TRAILER/CONTROL)     : a=29.43,  b=0.14715, c=0.0013734
//   PASSENGER_WAGON                 : a=29.43,  b=0.14715, c=0.0013734
//   FREIGHT_WAGON                   : a=14.715, b=0.07848, c=0.0007848
//   SERVICE_WAGON                   : a=24.525, b=0.12753, c=0.0011772
//
// Derived from UIC/PKP specific resistance formulae (Strahl method):
//   a_pt = a_spec × 1000 × 9.81   where a_spec is the dimensionless
//   b_pt = b_spec × 1000 × 9.81   rolling-resistance factor from
//   c_pt = c_spec × 1000 × 9.81   UIC 774-1 / ORE B12 tables.

struct DavisCoefficients
{
    float a = 0.0f;  // [N/t]
    float b = 0.0f;  // [N/(t·km/h)]
    float c = 0.0f;  // [N/(t·(km/h)²)]
};

// ── VehicleType ───────────────────────────────────────────────────────────────
// Physical properties shared by every unit of the same model.
// Loaded from data/vehicle_types/**/*.json (recursive scan).
// Directory layout: locomotive/{electric,diesel,steam}/, emu_unit/motor/,
//   dmu_unit/motor/, freight_wagon/hopper/, service_wagon/

struct VehicleType
{
    UID uid;
    std::string type_name;
    std::string pkp_series;  // e.g. ET22; empty when unknown
    std::string family;      // optional grouping label; empty when unknown
    std::string
        vehicle_type;  // LOCOMOTIVE | EMU_UNIT | PASSENGER_WAGON | FREIGHT_WAGON | SERVICE_WAGON
    std::string
        vehicle_subtype;  // ELECTRIC | DIESEL | STEAM | MOTOR | TRAILER | FLAT | COVERED | TANK | HOPPER
    float length_m;
    int axle_count;
    float mass_empty_t;
    std::optional<float> mass_gross_t;
    int max_speed_kmh;
    int braking_lambda_pct;
    std::optional<float> power_kw;
    std::optional<float> traction_force_kn;
    // For traction-capable type categories (LOCOMOTIVE, EMU/DMU MOTOR):
    //   true  -> type supports same-type multiple-unit traction coupling
    //   false -> type does not support coupling
    //   null  -> unknown/unverified capability
    std::optional<bool> multiple_coupling_capable;
    DavisCoefficients davis;  // resolved at load time (JSON or category default)
};

// ── Vehicle ───────────────────────────────────────────────────────────────────
// One numbered vehicle instance; type fields merged with instance overrides.
// After FleetRegistry::load() this struct holds fully resolved values —
// the engine never touches the raw JSON again.

struct Vehicle
{
    UID uid;
    std::string pid;  // operational number, e.g. "ET22-001"
    UID type_uid;
    std::string display_name;

    // ── Resolved properties ────────────────────────────────────────────────────
    std::string vehicle_type;
    std::string vehicle_subtype;
    float length_m;
    int axle_count;
    float mass_empty_t;
    float effective_mass_t;  // mass_gross_t if set, else mass_empty_t
    int max_speed_kmh;
    int braking_lambda_pct;
    std::optional<float> power_kw;
    std::optional<float> traction_force_kn;
    bool traction_capable = false;
    // Present only for traction-capable units.
    std::optional<TractionStatus> traction_status;
    // Copied from type; meaningful for traction-capable categories.
    std::optional<bool> multiple_coupling_capable;
    DavisCoefficients davis;
};

struct Carrier
{
    UID id;
    std::string name;
    std::vector<std::string> service_types;  // passenger | freight
    std::optional<std::string> logo;
};

// ── TrainConsist ──────────────────────────────────────────────────────────────
// An ordered sequence of Vehicle instances forming a complete trainset.
// Derived properties are computed once at load time (doc 10 formulae).

struct TrainConsist
{
    UID uid;
    std::string pid;
    std::string display_name;
    TrainCategory train_category;
    std::optional<UID> carrier_id;
    std::vector<UID> vehicle_uids;  // front → rear

    // ── Derived (computed at load time) ────────────────────────────────────────
    float total_length_m;
    int total_axles;
    float total_mass_t;
    float consist_lambda_pct;  // mass-weighted average braking percentage (UIC)
    float max_speed_kmh;       // min of all vehicles (weakest link)
    float total_traction_kn;   // sum of powered vehicles
    float total_power_kw;      // sum of powered vehicles
};

// ── Load error ────────────────────────────────────────────────────────────────

struct FleetLoadError : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

// ── FleetRegistry ─────────────────────────────────────────────────────────────
// Loads and caches all fleet data from the data/ directory tree.
// Thread-safety: load() is called once at session start on a single thread.
// After loading, all accessors are read-only and safe to call from any thread.

class FleetRegistry
{
    std::unordered_map<UID, VehicleType, std::hash<UID>> types_;
    std::unordered_map<UID, Vehicle, std::hash<UID>> vehicles_;
    std::unordered_map<UID, TrainConsist, std::hash<UID>> consists_;
    std::unordered_map<UID, Carrier, std::hash<UID>> carriers_;

public:
    // Load all three levels from the given data root directory.
    // Throws FleetLoadError on any JSON parse failure or broken reference.
    void load(const std::filesystem::path& data_root);

    // ── Accessors (read-only after load) ───────────────────────────────────────

    const VehicleType& get_type(UID uid) const;
    const Vehicle& get_vehicle(UID uid) const;
    const TrainConsist& get_consist(UID uid) const;

    bool has_vehicle(UID uid) const { return vehicles_.count(uid) > 0; }
    bool has_consist(UID uid) const { return consists_.count(uid) > 0; }
    bool has_carrier(UID id) const { return carriers_.count(id) > 0; }

    // Iteration support for snapshot building.
    const auto& all_consists() const { return consists_; }
    const auto& all_carriers() const { return carriers_; }

private:
    void load_types_(const std::filesystem::path& types_dir);
    void load_vehicles_(const std::filesystem::path& vehicles_dir);
    void load_consists_(const std::filesystem::path& consists_dir);
    void load_carriers_(const std::filesystem::path& carriers_file);

    static DavisCoefficients davis_defaults_(const std::string& vehicle_type,
                                             const std::string& vehicle_subtype, int max_speed_kmh);
};

}  // namespace engine::core
