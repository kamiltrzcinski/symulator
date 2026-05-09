#include "engine/sim/train_sim.hpp"

#include <gtest/gtest.h>

using namespace engine;

namespace
{

class FakePolicy final : public sim::ITrainControlPolicy
{
public:
    physics::DriverOutput next_output{};

    physics::DriverOutput compute(physics::DriverState prev_state,
                                  const physics::TrainPhysicsParams& params,
                                  const physics::TrainPhysicsState& state,
                                  const physics::DriverInput& input) const override
    {
        (void)prev_state;
        (void)params;
        (void)state;
        (void)input;
        return next_output;
    }
};

class FakeIntegrator final : public sim::IPhysicsIntegrator
{
public:
    mutable physics::TrainPhysicsState last_input_state{};
    mutable physics::TrainPhysicsParams last_input_params{};
    mutable float last_dt_s = 0.0f;

    physics::TrainPhysicsState next_state{};

    physics::TrainPhysicsState step(const physics::TrainPhysicsParams& params,
                                    const physics::TrainPhysicsState& state,
                                    float dt_s) const override
    {
        last_input_state = state;
        last_input_params = params;
        last_dt_s = dt_s;
        return next_state;
    }
};

class FakeEventSink final : public sim::ITrainEventSink
{
public:
    int crossing_count = 0;
    core::GID last_train_gid{};
    sim::SectionCrossing last_crossing{};

    void on_section_crossing(const core::GID& train_gid,
                             const sim::SectionCrossing& crossing) override
    {
        crossing_count += 1;
        last_train_gid = train_gid;
        last_crossing = crossing;
    }
};

}  // namespace

TEST(TrainSim, MakeTrainSimStateAggregatesVehicles)
{
    core::TrainConsist consist{};
    consist.gid = core::GID{"TRN-TRJ-TEST-0000001"};
    consist.consist_lambda_pct = 100.0f;
    consist.vehicle_gids = {core::GID{"VEH-1"}, core::GID{"VEH-2"}};

    core::Vehicle v1{};
    v1.gid = core::GID{"VEH-1"};
    v1.effective_mass_t = 80.0f;
    v1.max_speed_kmh = 120;
    v1.traction_force_kn = 280.0f;
    v1.davis = core::DavisCoefficients{39.24f, 0.1962f, 0.0017658f};

    core::Vehicle v2{};
    v2.gid = core::GID{"VEH-2"};
    v2.effective_mass_t = 20.0f;
    v2.max_speed_kmh = 100;
    v2.traction_force_kn = std::nullopt;
    v2.davis = core::DavisCoefficients{14.715f, 0.07848f, 0.0007848f};

    const sim::TrainSimState state = sim::make_train_sim_state(
        consist, std::vector<core::Vehicle>{v1, v2}, core::GID{"OT-TEST-001"});

    EXPECT_NEAR(state.physics_params.total_mass_t, 100.0f, 0.001f);
    EXPECT_NEAR(state.physics_params.max_traction_kn, 280.0f, 0.001f);
    EXPECT_NEAR(state.physics_params.max_speed_ms, 100.0f / 3.6f, 0.001f);
    EXPECT_GT(state.max_brake_kn, 0.0f);
}

TEST(TrainSim, TickUsesInjectedPolicyAndIntegrator)
{
    auto policy = std::make_shared<FakePolicy>();
    policy->next_output = physics::DriverOutput{50.0f, 0.0f, physics::DriverState::ACCELERATING};

    auto integrator = std::make_shared<FakeIntegrator>();
    integrator->next_state = physics::TrainPhysicsState{};
    integrator->next_state.position_m = 5.0f;
    integrator->next_state.velocity_ms = 2.0f;
    integrator->next_state.accel_ms2 = 0.4f;

    auto sink = std::make_shared<FakeEventSink>();

    sim::TrainSimState initial{};
    initial.train_gid = core::GID{"TRN-1"};
    initial.current_section_gid = core::GID{"OT-A"};
    initial.physics_params =
        physics::TrainPhysicsParams{100.0f, 300.0f, 30.0f, 1000.0f, 1.0f, 0.01f};
    initial.max_brake_kn = 120.0f;

    sim::TrainSim sim_train(initial, policy, integrator, sink);

    sim::TrainSimTickInput in{};
    in.driver_input = physics::DriverInput{core::SignalAspect::S2_PROCEED, 1000.0f, 30.0f, 120.0f,
                                           core::SignalAspect::S2_PROCEED, 1000.0f};
    in.section_length_m = 100.0f;

    const auto out = sim_train.tick(0.05f, in);

    EXPECT_NEAR(integrator->last_input_state.traction_kn, 50.0f, 0.001f);
    EXPECT_NEAR(integrator->last_input_state.brake_kn, 0.0f, 0.001f);
    EXPECT_NEAR(integrator->last_dt_s, 0.05f, 1e-6f);
    EXPECT_EQ(out.state.driver_state, physics::DriverState::ACCELERATING);
    EXPECT_NEAR(out.state.physics_state.position_m, 5.0f, 0.001f);
    EXPECT_FALSE(out.crossing.has_value());
}

TEST(TrainSim, EmitsSectionCrossingWhenBoundaryIsPassed)
{
    auto policy = std::make_shared<FakePolicy>();
    policy->next_output = physics::DriverOutput{0.0f, 0.0f, physics::DriverState::CRUISING};

    auto integrator = std::make_shared<FakeIntegrator>();
    integrator->next_state = physics::TrainPhysicsState{};
    integrator->next_state.position_m = 105.0f;
    integrator->next_state.velocity_ms = 10.0f;

    auto sink = std::make_shared<FakeEventSink>();

    sim::TrainSimState initial{};
    initial.train_gid = core::GID{"TRN-1"};
    initial.current_section_gid = core::GID{"OT-A"};
    initial.physics_params =
        physics::TrainPhysicsParams{100.0f, 300.0f, 30.0f, 1000.0f, 1.0f, 0.01f};
    initial.max_brake_kn = 120.0f;

    sim::TrainSim sim_train(initial, policy, integrator, sink);

    sim::TrainSimTickInput in{};
    in.driver_input = physics::DriverInput{core::SignalAspect::S2_PROCEED, 1000.0f, 30.0f, 120.0f,
                                           core::SignalAspect::S2_PROCEED, 1000.0f};
    in.section_length_m = 100.0f;
    in.next_section_gid = core::GID{"OT-B"};

    const auto out = sim_train.tick(0.05f, in);

    ASSERT_TRUE(out.crossing.has_value());
    EXPECT_EQ(out.crossing->from_section_gid.value, "OT-A");
    EXPECT_EQ(out.crossing->to_section_gid.value, "OT-B");
    EXPECT_NEAR(out.crossing->overshoot_m, 5.0f, 0.001f);
    EXPECT_EQ(out.state.current_section_gid.value, "OT-B");
    EXPECT_NEAR(out.state.physics_state.position_m, 5.0f, 0.001f);

    EXPECT_EQ(sink->crossing_count, 1);
    EXPECT_EQ(sink->last_train_gid.value, "TRN-1");
}

TEST(TrainSim, StopsAtDeadEndWhenNoNextSectionProvided)
{
    auto policy = std::make_shared<FakePolicy>();
    policy->next_output = physics::DriverOutput{100.0f, 0.0f, physics::DriverState::CRUISING};

    auto integrator = std::make_shared<FakeIntegrator>();
    integrator->next_state = physics::TrainPhysicsState{};
    integrator->next_state.position_m = 120.0f;
    integrator->next_state.velocity_ms = 8.0f;

    auto sink = std::make_shared<FakeEventSink>();

    sim::TrainSimState initial{};
    initial.train_gid = core::GID{"TRN-1"};
    initial.current_section_gid = core::GID{"OT-A"};
    initial.physics_params =
        physics::TrainPhysicsParams{100.0f, 300.0f, 30.0f, 1000.0f, 1.0f, 0.01f};
    initial.max_brake_kn = 120.0f;

    sim::TrainSim sim_train(initial, policy, integrator, sink);

    sim::TrainSimTickInput in{};
    in.driver_input = physics::DriverInput{core::SignalAspect::S2_PROCEED, 1000.0f, 30.0f, 120.0f,
                                           core::SignalAspect::S2_PROCEED, 1000.0f};
    in.section_length_m = 100.0f;

    const auto out = sim_train.tick(0.05f, in);

    EXPECT_FALSE(out.crossing.has_value());
    EXPECT_NEAR(out.state.physics_state.position_m, 100.0f, 0.001f);
    EXPECT_NEAR(out.state.physics_state.velocity_ms, 0.0f, 0.001f);
    EXPECT_EQ(out.state.driver_state, physics::DriverState::STOPPED);
    EXPECT_EQ(sink->crossing_count, 0);
}
