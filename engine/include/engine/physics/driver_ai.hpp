#pragma once

#include <cstdint>

#include "engine/core/types.hpp"
#include "engine/physics/physics_model.hpp"

// ── DriverAI — deterministic locomotive control automaton ─────────────────────
// Reads the current signal aspect and train physics state; produces target
// traction/brake force commands for the next simulation tick.
//
// State machine:
//
//   ┌─────────────┐
//   │   STOPPED   │◄──── velocity = 0, signal = STOP
//   └──────┬──────┘
//          │ signal clears AND target_speed > 0
//          ▼
//   ┌─────────────┐
//   │ ACCELERATING│  traction = max, brake = 0
//   └──────┬──────┘
//          │ velocity ≥ target_speed
//          ▼
//   ┌─────────────┐
//   │  CRUISING   │  traction = resistance force only, brake = 0
//   └──────┬──────┘
//          │ braking_distance ≥ distance_to_target_stop
//          ▼
//   ┌─────────────┐
//   │   BRAKING   │  traction = 0, brake = max
//   └─────────────┘
//
// Speed-restricted aspects (S3_PROCEED_40 etc.) set target_speed to the
// restricted value and trigger braking if current speed exceeds it.
//
// No memory of previous aspect; purely reactive to current inputs.

namespace engine::physics
{

// ── DriverState ───────────────────────────────────────────────────────────────
enum class DriverState : std::uint8_t
{
    STOPPED,
    ACCELERATING,
    CRUISING,
    BRAKING,
};

// ── DriverInput ───────────────────────────────────────────────────────────────
// Everything the DriverAI needs to decide its next output.
struct DriverInput
{
    engine::core::SignalAspect aspect;  // current signal ahead
    float distance_to_signal_m;         // metres until the signal mast
    float target_speed_ms;              // consist speed limit [m/s]
    float max_brake_kn;                 // consist full-service brake force [kN]
};

// ── DriverOutput ──────────────────────────────────────────────────────────────
// Forces the DriverAI requests for the next tick.
struct DriverOutput
{
    float traction_kn = 0.0f;  // requested traction force [kN], 0–max
    float brake_kn = 0.0f;     // requested brake force    [kN], 0–max
    DriverState state = DriverState::STOPPED;
};

// ── SignalAspect → speed limit mapping ────────────────────────────────────────
// Returns the maximum permitted speed for the given signal aspect in m/s.
// STOP aspects return 0.0, permissive aspects return a large sentinel (999).

inline constexpr float aspect_speed_ms(engine::core::SignalAspect asp) noexcept
{
    using SA = engine::core::SignalAspect;
    switch (asp)
    {
        case SA::S1_STOP:
            return 0.0f;
        case SA::S2_PROCEED:
            return 999.0f;
        case SA::S3_PROCEED_40:
            return 40.0f / 3.6f;
        case SA::S4_PROCEED_40_EXPECT_STOP:
            return 40.0f / 3.6f;
        case SA::S5_EXPECT_STOP:
            return 999.0f;  // advisory only
        case SA::S6_PROCEED_100:
            return 100.0f / 3.6f;
        case SA::S7_PROCEED_100_EXPECT_STOP:
            return 100.0f / 3.6f;
        case SA::S8_PROCEED_100_EXPECT_40:
            return 100.0f / 3.6f;
        case SA::S9_PROCEED_100_EXPECT_60:
            return 100.0f / 3.6f;
        case SA::S10_PROCEED_40:
            return 40.0f / 3.6f;
        case SA::S11_PROCEED_40_EXPECT_40:
            return 40.0f / 3.6f;
        case SA::S12_PROCEED_60:
            return 60.0f / 3.6f;
        case SA::S13_PROCEED_60_EXPECT_60:
            return 60.0f / 3.6f;
        case SA::MS2_SHUNTING_ALLOWED:
            return 40.0f / 3.6f;
        default:
            return 0.0f;  // safe default
    }
}

// ── Is this a STOP aspect? ─────────────────────────────────────────────────
inline constexpr bool is_stop_aspect(engine::core::SignalAspect asp) noexcept
{
    return asp == engine::core::SignalAspect::S1_STOP;
}

// ── DriverAI ──────────────────────────────────────────────────────────────────
// Pure, stateless compute function.  Caller maintains DriverState externally.
// Call once per simulation tick; returns the updated DriverOutput.

class DriverAI
{
public:
    // Tolerance margin added to braking distance (safety overhead, 10 %).
    static constexpr float kBrakingMargin = 1.10f;

    // Speed band within which we consider the train "at cruise speed".
    // If v > target − kSpeedBand, don't apply more traction.
    static constexpr float kSpeedBandMs = 0.5f;  // 0.5 m/s ≈ 1.8 km/h

    // Velocity below which we declare the train stopped [m/s].
    static constexpr float kStoppedThresholdMs = 0.05f;

    // ── Main entry point ───────────────────────────────────────────────────────
    // prev_state  : DriverState from the previous tick
    // p           : immutable consist physics params
    // s           : current physics state (position, velocity, ...)
    // inp         : current driver inputs (signal, distances, speed limit)
    // Returns     : new DriverOutput (traction_kn, brake_kn, new DriverState)

    static DriverOutput tick(DriverState prev_state, const TrainPhysicsParams& p,
                             const TrainPhysicsState& s, const DriverInput& inp) noexcept
    {
        DriverOutput out{};

        // ── Effective speed limit (min of consist limit and signal limit) ─────
        const float signal_vmax = aspect_speed_ms(inp.aspect);
        const float eff_vmax_ms =
            (signal_vmax < inp.target_speed_ms) ? signal_vmax : inp.target_speed_ms;

        // ── STOP ─────────────────────────────────────────────────────────────
        // Must stop at or before the signal mast.
        const bool must_stop = is_stop_aspect(inp.aspect);

        // ── Braking distance needed to reach 0 m/s ────────────────────────────
        // Use full service brake.
        TrainPhysicsState s_for_brake = s;
        s_for_brake.brake_kn = inp.max_brake_kn;
        const float d_stop = PhysicsModel::braking_distance(p, s_for_brake) * kBrakingMargin;

        // ── State transitions ─────────────────────────────────────────────────
        DriverState new_state = prev_state;

        if (must_stop)
        {
            // Need to stop before signal
            if (s.velocity_ms <= kStoppedThresholdMs)
                new_state = DriverState::STOPPED;
            else if (d_stop >= inp.distance_to_signal_m)
                new_state = DriverState::BRAKING;
            else
                new_state = DriverState::BRAKING;  // already committed to stopping
        }
        else if (s.velocity_ms <= kStoppedThresholdMs && prev_state == DriverState::STOPPED)
        {
            // At rest, signal permits movement
            if (eff_vmax_ms > kStoppedThresholdMs)
                new_state = DriverState::ACCELERATING;
            // else stay STOPPED
        }
        else if (prev_state == DriverState::STOPPED && eff_vmax_ms > kStoppedThresholdMs)
        {
            new_state = DriverState::ACCELERATING;
        }
        else if (prev_state == DriverState::ACCELERATING)
        {
            if (s.velocity_ms >= eff_vmax_ms - kSpeedBandMs)
                new_state = DriverState::CRUISING;
            // stay ACCELERATING if we haven't reached target yet
        }
        else if (prev_state == DriverState::CRUISING)
        {
            if (s.velocity_ms > eff_vmax_ms + kSpeedBandMs)
                new_state = DriverState::BRAKING;
            else if (s.velocity_ms < eff_vmax_ms - kSpeedBandMs * 2.0f)
                new_state = DriverState::ACCELERATING;
            // else stay CRUISING
        }
        else if (prev_state == DriverState::BRAKING)
        {
            if (s.velocity_ms <= kStoppedThresholdMs && must_stop)
                new_state = DriverState::STOPPED;
            else if (!must_stop && s.velocity_ms <= eff_vmax_ms - kSpeedBandMs)
                new_state = DriverState::CRUISING;
            // else stay BRAKING
        }

        // ── Force commands per state ──────────────────────────────────────────
        switch (new_state)
        {
            case DriverState::STOPPED:
                out.traction_kn = 0.0f;
                out.brake_kn = inp.max_brake_kn;  // hold brakes applied
                break;

            case DriverState::ACCELERATING:
                out.traction_kn = p.max_traction_kn;
                out.brake_kn = 0.0f;
                break;

            case DriverState::CRUISING:
            {
                // Apply only enough traction to overcome Davis resistance.
                // This keeps speed roughly constant without oscillation.
                const float f_res = PhysicsModel::davis_resistance(p, s.velocity_ms);
                out.traction_kn = f_res / 1000.0f;  // kN
                out.brake_kn = 0.0f;
                break;
            }

            case DriverState::BRAKING:
                out.traction_kn = 0.0f;
                out.brake_kn = inp.max_brake_kn;
                break;
        }

        out.state = new_state;
        return out;
    }
};

}  // namespace engine::physics
