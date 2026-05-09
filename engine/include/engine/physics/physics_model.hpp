#pragma once

#include <algorithm>
#include <cstdint>

// ── Train physics model ───────────────────────────────────────────────────────
// Newton's second law integration for train longitudinal motion.
//
// Resistance model (Davis formula, per-tonne coefficients):
//   F_res [N] = (davis_a + davis_b·v_kmh + davis_c·v_kmh²) · mass_t
//
// Equation of motion (single tick):
//   F_net = F_traction − F_resistance − F_brake
//   a = F_net / (mass_t · 1000)          [m/s²]
//   v_new = clamp(v + a · dt, 0, v_max)  [m/s]
//   x_new = x + v · dt + 0.5 · a · dt²  [m]
//
// All internal calculations use SI units.
// Public API surfaces velocities in both m/s and km/h for convenience.

namespace engine::physics
{

// ── Davis resistance coefficients ─────────────────────────────────────────────
// Match fleet_registry.hpp DavisCoefficients exactly.
struct DavisCoeff
{
    float a = 39.24f;      // [N/t]  — constant rolling resistance
    float b = 0.1962f;     // [N/(t·km/h)]
    float c = 0.0017658f;  // [N/(t·(km/h)²)]
};

// ── TrainPhysicsParams ────────────────────────────────────────────────────────
// Immutable consist-level parameters resolved once at train formation.
struct TrainPhysicsParams
{
    float total_mass_t = 0.0f;     // total consist mass [t]
    float max_traction_kn = 0.0f;  // maximum starting traction force [kN]
    float max_speed_ms = 999.0f;   // consist speed limit [m/s]

    // Davis coefficients for the whole consist.
    // For a multi-vehicle consist, sum individual davis * mass_t contributions:
    //   A_consist = Σ (a_i * m_i)   [N]
    //   B_consist = Σ (b_i * m_i)   [N/(km/h)]
    //   C_consist = Σ (c_i * m_i)   [N/(km/h)²]
    // These are stored as absolute (pre-multiplied by mass) values.
    float davis_A = 0.0f;  // [N]
    float davis_B = 0.0f;  // [N/(km/h)]
    float davis_C = 0.0f;  // [N/(km/h)²]
};

// ── TrainPhysicsState ─────────────────────────────────────────────────────────
// Mutable per-tick state for one train.
struct TrainPhysicsState
{
    float position_m = 0.0f;   // nose position along current section [m]
    float velocity_ms = 0.0f;  // current speed [m/s]
    float accel_ms2 = 0.0f;    // acceleration from last tick [m/s²]
    float traction_kn = 0.0f;  // applied traction force [kN]  (0–max)
    float brake_kn = 0.0f;     // applied brake force [kN]     (0–max)
};

// ── PhysicsModel ──────────────────────────────────────────────────────────────
// Pure, stateless computation. No heap allocation. No exceptions.
// Call integrate() once per simulation tick.

class PhysicsModel
{
public:
    // ── Davis running resistance ───────────────────────────────────────────────
    // F_res [N] given v_ms in m/s.
    // Internally converts to km/h for the Davis formula.
    static constexpr float davis_resistance(const TrainPhysicsParams& p, float v_ms) noexcept
    {
        const float v_kmh = v_ms * 3.6f;
        return p.davis_A + p.davis_B * v_kmh + p.davis_C * v_kmh * v_kmh;
    }

    // ── Braking distance (stopping distance from v_ms at full service brake) ──
    // Returns the distance [m] needed to stop from v_ms using the current
    // brake force as set in state.brake_kn.  Used by DriverAI to decide when
    // to begin braking.
    //
    // Approximation: constant deceleration (ignores Davis resistance term during
    // braking — conservative, i.e. slightly over-estimates distance).
    //   d = v² / (2 · a_brake)
    static constexpr float braking_distance(const TrainPhysicsParams& p,
                                            const TrainPhysicsState& s) noexcept
    {
        const float mass_kg = p.total_mass_t * 1000.0f;
        const float brake_n = s.brake_kn * 1000.0f;
        if (brake_n <= 0.0f || mass_kg <= 0.0f)
            return 1e9f;                              // can't brake → infinite distance
        const float a_brake_ms2 = brake_n / mass_kg;  // deceleration [m/s²]
        return (s.velocity_ms * s.velocity_ms) / (2.0f * a_brake_ms2);
    }

    // ── Single-tick integration (Forward Euler with velocity clamping) ─────────
    // dt_s : simulation tick in seconds (e.g. 0.05 for 20 Hz)
    // Returns the updated state.  Input state is not modified.
    //
    // Sign convention:
    //   traction_kn ≥ 0   (drives train forward)
    //   brake_kn    ≥ 0   (retards train)
    //   Davis resistance is always opposing motion (subtracted from traction)
    static TrainPhysicsState integrate(const TrainPhysicsParams& p, const TrainPhysicsState& s,
                                       float dt_s) noexcept
    {
        TrainPhysicsState next = s;

        const float mass_kg = p.total_mass_t * 1000.0f;
        const float f_traction = std::min(s.traction_kn, p.max_traction_kn) * 1000.0f;  // N
        const float f_brake = s.brake_kn * 1000.0f;                                     // N
        const float f_resist = davis_resistance(p, s.velocity_ms);                      // N

        // Net force (forward positive)
        float f_net = f_traction - f_resist - f_brake;

        // Prevent overshoot: if already at rest and net force is negative, clamp
        if (s.velocity_ms <= 0.0f && f_net < 0.0f)
            f_net = 0.0f;

        const float accel = f_net / mass_kg;  // m/s²
        float v_new = s.velocity_ms + accel * dt_s;
        v_new = std::clamp(v_new, 0.0f, p.max_speed_ms);

        // Leapfrog-style position update (use average velocity)
        const float v_avg = 0.5f * (s.velocity_ms + v_new);
        next.position_m = s.position_m + v_avg * dt_s;
        next.velocity_ms = v_new;
        next.accel_ms2 = accel;

        return next;
    }

    // ── Maximum brake deceleration [m/s²] ────────────────────────────────────
    // Derived from consist braking percentage (UIC lambda method).
    //   a_brake = lambda_pct / 100 * g         [m/s²]  (UIC simplified)
    //
    // UIC braking distance rule gives lambda as the fraction of total weight
    // that the brake can exert.  For simulation, clamp to [0.3, 1.2] m/s².
    static constexpr float brake_decel_from_lambda(float lambda_pct) noexcept
    {
        // UIC formula: I_brake = lambda/100 × W_brake [kN], W_brake = m×g
        // Simplified: a_brake ≈ lambda/100 × g × 0.85 (empirical factor)
        constexpr float g = 9.81f;
        return std::clamp(lambda_pct / 100.0f * g * 0.85f, 0.3f, 8.0f);
    }

    // ── Maximum brake force [kN] from consist params ──────────────────────────
    static constexpr float max_brake_kn(const TrainPhysicsParams& p, float lambda_pct) noexcept
    {
        return brake_decel_from_lambda(lambda_pct) * p.total_mass_t;  // kN
    }
};

// ── Helper: build TrainPhysicsParams for a consist ────────────────────────────
// Aggregates individual vehicle contributions into consist-level parameters.
// Call once when a consist is formed or modified.

struct VehiclePhysicsContrib
{
    float mass_t = 0.0f;
    float traction_kn = 0.0f;  // 0 for non-traction vehicles
    float max_speed_ms = 999.0f;
    float davis_a = 0.0f;  // per-tonne [N/t]
    float davis_b = 0.0f;  // per-tonne [N/(t·km/h)]
    float davis_c = 0.0f;  // per-tonne [N/(t·(km/h)²)]
};

inline TrainPhysicsParams build_train_params(const VehiclePhysicsContrib* vehicles,
                                             std::size_t count)
{
    TrainPhysicsParams p{};
    for (std::size_t i = 0; i < count; ++i)
    {
        const auto& v = vehicles[i];
        p.total_mass_t += v.mass_t;
        p.max_traction_kn += v.traction_kn;
        p.max_speed_ms = std::min(p.max_speed_ms, v.max_speed_ms);
        // Pre-multiply Davis coefficients by vehicle mass for consist totals
        p.davis_A += v.davis_a * v.mass_t;  // N
        p.davis_B += v.davis_b * v.mass_t;  // N/(km/h)
        p.davis_C += v.davis_c * v.mass_t;  // N/(km/h)²
    }
    if (p.max_speed_ms >= 999.0f)
        p.max_speed_ms = 0.0f;
    return p;
}

}  // namespace engine::physics
