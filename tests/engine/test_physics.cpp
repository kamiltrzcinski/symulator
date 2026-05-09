#include "engine/physics/driver_ai.hpp"
#include "engine/physics/physics_model.hpp"

#include <gtest/gtest.h>
#include <algorithm>

using namespace engine::physics;
using namespace engine::core;

// ── Helpers ───────────────────────────────────────────────────────────────────

// EU07 consist: single locomotive, 80t, 280 kN traction, 125 km/h
static TrainPhysicsParams eu07_params()
{
    TrainPhysicsParams p{};
    p.total_mass_t = 80.0f;
    p.max_traction_kn = 280.0f;
    p.max_speed_ms = 125.0f / 3.6f;  // 34.722 m/s
    // Davis (per-tonne × mass for EU07):
    p.davis_A = 39.24f * 80.0f;      // 3139.2 N
    p.davis_B = 0.1962f * 80.0f;     // 15.696 N/(km/h)
    p.davis_C = 0.0017658f * 80.0f;  // 0.141264 N/(km/h)²
    return p;
}

static DriverInput make_proceed_input(float target_ms = 999.0f, float dist_m = 10000.0f,
                                      float brake_kn = 150.0f,
                                      SignalAspect next_aspect = SignalAspect::S2_PROCEED,
                                      float dist_to_next_m = 1.0e9f)
{
    return DriverInput{
        SignalAspect::S2_PROCEED, dist_m, target_ms, brake_kn, next_aspect, dist_to_next_m};
}

// ── PhysicsModel tests ────────────────────────────────────────────────────────

TEST(PhysicsModel, DavisResistanceAtRest)
{
    auto p = eu07_params();
    float r = PhysicsModel::davis_resistance(p, 0.0f);
    // At v=0: F_res = davis_A = 3139.2 N
    EXPECT_NEAR(r, 3139.2f, 1.0f);
}

TEST(PhysicsModel, DavisResistanceAt125kmh)
{
    auto p = eu07_params();
    float r = PhysicsModel::davis_resistance(p, 125.0f / 3.6f);
    // F = 3139.2 + 15.696*125 + 0.141264*125² ≈ 7308 N
    EXPECT_NEAR(r, 7308.2f, 10.0f);
}

TEST(PhysicsModel, EU07SelfPowerLessThan50Percent)
{
    auto p = eu07_params();
    float v_ms = 125.0f / 3.6f;
    float r = PhysicsModel::davis_resistance(p, v_ms);
    float power_w = r * v_ms;
    // EU07 has 2000 kW → self-resistance < 1000 kW
    EXPECT_LT(power_w, 1000.0f * 1000.0f);
}

TEST(PhysicsModel, IntegrationFromRestAccelerates)
{
    auto p = eu07_params();
    TrainPhysicsState s{};
    s.traction_kn = 280.0f;

    for (int i = 0; i < 10; ++i)
        s = PhysicsModel::integrate(p, s, 0.05f);

    EXPECT_GT(s.velocity_ms, 0.0f);
    EXPECT_GT(s.position_m, 0.0f);
    EXPECT_LE(s.velocity_ms, p.max_speed_ms);
}

TEST(PhysicsModel, FullBrakeFrom50kmhStops)
{
    auto p = eu07_params();
    TrainPhysicsState s{};
    s.velocity_ms = 50.0f / 3.6f;
    s.brake_kn = 120.0f;

    int ticks = 0;
    while (s.velocity_ms > 0.05f && ticks < 600)
    {
        s = PhysicsModel::integrate(p, s, 0.05f);
        ++ticks;
    }
    EXPECT_LE(s.velocity_ms, 0.05f);
    EXPECT_LT(ticks, 600);
}

TEST(PhysicsModel, SpeedClampedAtMaxSpeed)
{
    auto p = eu07_params();
    TrainPhysicsState s{};
    s.velocity_ms = p.max_speed_ms;
    s.traction_kn = 280.0f;

    auto next = PhysicsModel::integrate(p, s, 0.05f);
    EXPECT_LE(next.velocity_ms, p.max_speed_ms + 0.001f);
}

TEST(PhysicsModel, StaticTrainWithBrakeNoNegativeVelocity)
{
    auto p = eu07_params();
    TrainPhysicsState s{};
    s.velocity_ms = 0.0f;
    s.brake_kn = 200.0f;

    auto next = PhysicsModel::integrate(p, s, 0.05f);
    EXPECT_GE(next.velocity_ms, 0.0f);
    EXPECT_GE(next.position_m, -0.001f);
}

TEST(PhysicsModel, BuildTrainParamsAggregatesTwoLocos)
{
    VehiclePhysicsContrib v[2];
    for (auto& c : v)
    {
        c.mass_t = 80.0f;
        c.traction_kn = 280.0f;
        c.max_speed_ms = 125.0f / 3.6f;
        c.davis_a = 39.24f;
        c.davis_b = 0.1962f;
        c.davis_c = 0.0017658f;
    }

    auto p = build_train_params(v, 2);
    EXPECT_NEAR(p.total_mass_t, 160.0f, 0.001f);
    EXPECT_NEAR(p.max_traction_kn, 560.0f, 0.001f);
    EXPECT_NEAR(p.davis_A, 39.24f * 160.0f, 1.0f);
}

TEST(PhysicsModel, BrakingDistanceFrom100kmh)
{
    auto p = eu07_params();
    TrainPhysicsState s{};
    s.velocity_ms = 100.0f / 3.6f;  // 27.78 m/s
    s.brake_kn = 150.0f;

    float d = PhysicsModel::braking_distance(p, s);
    // a_brake = 150000 / 80000 = 1.875 m/s²
    // d = 27.78² / (2 * 1.875) ≈ 205.8 m
    EXPECT_NEAR(d, 205.8f, 2.0f);
}

// ── DriverAI tests ────────────────────────────────────────────────────────────

TEST(DriverAI, StoppedAtStopSignalStaysStopped)
{
    auto p = eu07_params();
    TrainPhysicsState s{};
    DriverInput inp{SignalAspect::S1_STOP, 500.0f, 999.0f, 150.0f};

    auto out = DriverAI::tick(DriverState::STOPPED, p, s, inp);
    EXPECT_EQ(out.state, DriverState::STOPPED);
    EXPECT_FLOAT_EQ(out.traction_kn, 0.0f);
    EXPECT_GT(out.brake_kn, 0.0f);
}

TEST(DriverAI, ProceedFromRestTriggersAcceleration)
{
    auto p = eu07_params();
    TrainPhysicsState s{};
    auto inp = make_proceed_input(125.0f / 3.6f);

    auto out = DriverAI::tick(DriverState::STOPPED, p, s, inp);
    EXPECT_EQ(out.state, DriverState::ACCELERATING);
    EXPECT_NEAR(out.traction_kn, p.max_traction_kn, 0.001f);
    EXPECT_FLOAT_EQ(out.brake_kn, 0.0f);
}

TEST(DriverAI, AtTargetSpeedTransitionsToCruising)
{
    auto p = eu07_params();
    TrainPhysicsState s{};
    s.velocity_ms = 100.0f / 3.6f;

    auto inp = make_proceed_input(100.0f / 3.6f);
    auto out = DriverAI::tick(DriverState::ACCELERATING, p, s, inp);

    EXPECT_EQ(out.state, DriverState::CRUISING);
    EXPECT_FLOAT_EQ(out.brake_kn, 0.0f);
    float r_kn = PhysicsModel::davis_resistance(p, s.velocity_ms) / 1000.0f;
    EXPECT_NEAR(out.traction_kn, r_kn, 0.001f);
}

TEST(DriverAI, StopSignalCloseAheadTriggersBraking)
{
    auto p = eu07_params();
    TrainPhysicsState s{};
    s.velocity_ms = 80.0f / 3.6f;  // 22.2 m/s

    // braking_distance ≈ 131.5 m, signal 100 m away → must brake
    DriverInput inp{SignalAspect::S1_STOP, 100.0f, 999.0f, 150.0f};

    auto out = DriverAI::tick(DriverState::CRUISING, p, s, inp);
    EXPECT_EQ(out.state, DriverState::BRAKING);
    EXPECT_FLOAT_EQ(out.traction_kn, 0.0f);
    EXPECT_NEAR(out.brake_kn, 150.0f, 0.001f);
}

TEST(DriverAI, StopSignalFarAheadDoesNotForceImmediateBraking)
{
    auto p = eu07_params();
    TrainPhysicsState s{};
    s.velocity_ms = 80.0f / 3.6f;

    // Red signal is far away: train should continue running and brake later.
    DriverInput inp{SignalAspect::S1_STOP,    5000.0f, 125.0f / 3.6f, 150.0f,
                    SignalAspect::S2_PROCEED, 1.0e9f};
    auto out = DriverAI::tick(DriverState::CRUISING, p, s, inp);

    EXPECT_NE(out.state, DriverState::BRAKING);
    EXPECT_FLOAT_EQ(out.brake_kn, 0.0f);
}

TEST(DriverAI, WarningExpectStopTriggersProactiveBraking)
{
    auto p = eu07_params();
    TrainPhysicsState s{};
    s.velocity_ms = 100.0f / 3.6f;

    // Current signal allows proceed; next signal is expected STOP and close enough.
    auto inp =
        make_proceed_input(125.0f / 3.6f, 5000.0f, 150.0f, SignalAspect::S5_EXPECT_STOP, 120.0f);
    auto out = DriverAI::tick(DriverState::CRUISING, p, s, inp);

    EXPECT_EQ(out.state, DriverState::BRAKING);
    EXPECT_NEAR(out.brake_kn, 150.0f, 0.001f);
}

TEST(DriverAI, WarningExpectStopFarAwayKeepsCruising)
{
    auto p = eu07_params();
    TrainPhysicsState s{};
    s.velocity_ms = 100.0f / 3.6f;

    auto inp =
        make_proceed_input(125.0f / 3.6f, 5000.0f, 150.0f, SignalAspect::S5_EXPECT_STOP, 5000.0f);
    auto out = DriverAI::tick(DriverState::CRUISING, p, s, inp);

    EXPECT_NE(out.state, DriverState::BRAKING);
}

TEST(DriverAI, ProceedAt40LimitsViaS3Aspect)
{
    auto p = eu07_params();
    TrainPhysicsState s{};
    s.velocity_ms = 80.0f / 3.6f;  // over 40 km/h limit

    DriverInput inp{SignalAspect::S3_PROCEED_40, 5000.0f, 125.0f / 3.6f, 150.0f};
    auto out = DriverAI::tick(DriverState::CRUISING, p, s, inp);

    EXPECT_EQ(out.state, DriverState::BRAKING);
}

TEST(DriverAI, FullSimulationAccelerateCruiseBrake)
{
    auto p = eu07_params();
    TrainPhysicsState s{};
    DriverState ds = DriverState::STOPPED;

    // Phase 1: accelerate to 100 km/h and cruise
    auto inp_proceed = make_proceed_input(100.0f / 3.6f, 5000.0f, 150.0f);
    bool reached_cruise = false;
    for (int i = 0; i < 2000 && !reached_cruise; ++i)
    {
        auto out = DriverAI::tick(ds, p, s, inp_proceed);
        s.traction_kn = out.traction_kn;
        s.brake_kn = out.brake_kn;
        s = PhysicsModel::integrate(p, s, 0.05f);
        ds = out.state;
        if (ds == DriverState::CRUISING)
            reached_cruise = true;
    }
    EXPECT_TRUE(reached_cruise);
    EXPECT_NEAR(s.velocity_ms, 100.0f / 3.6f, 1.0f);

    // Phase 2: STOP signal 300 m ahead
    float d = 300.0f;
    bool stopped = false;
    for (int i = 0; i < 2000 && !stopped; ++i)
    {
        DriverInput inp{SignalAspect::S1_STOP, d, 999.0f, 150.0f};
        auto out = DriverAI::tick(ds, p, s, inp);
        s.traction_kn = out.traction_kn;
        s.brake_kn = out.brake_kn;
        float v_prev = s.velocity_ms;
        s = PhysicsModel::integrate(p, s, 0.05f);
        d = std::max(0.0f, d - 0.5f * (v_prev + s.velocity_ms) * 0.05f);
        ds = out.state;
        if (ds == DriverState::STOPPED)
            stopped = true;
    }
    EXPECT_TRUE(stopped);
    EXPECT_LE(s.velocity_ms, DriverAI::kStoppedThresholdMs + 0.01f);
}
