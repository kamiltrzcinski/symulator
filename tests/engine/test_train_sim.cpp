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
    core::UID last_train_uid{};
    sim::SectionCrossing last_crossing{};

    void on_section_crossing(core::UID train_uid, const sim::SectionCrossing& crossing) override
    {
        crossing_count += 1;
        last_train_uid = train_uid;
        last_crossing = crossing;
    }
};

constexpr core::UID kTrain1 =
    core::make_uid(core::UIDDomain::ROLLING_STOCK, core::UIDKind::TRAIN_CONSIST, 0, 1);
constexpr core::UID kTrain2 =
    core::make_uid(core::UIDDomain::ROLLING_STOCK, core::UIDKind::TRAIN_CONSIST, 0, 2);
constexpr core::UID kTrain3 =
    core::make_uid(core::UIDDomain::ROLLING_STOCK, core::UIDKind::TRAIN_CONSIST, 0, 3);
constexpr core::UID kTrain4 =
    core::make_uid(core::UIDDomain::ROLLING_STOCK, core::UIDKind::TRAIN_CONSIST, 0, 4);
constexpr core::UID kVeh1 =
    core::make_uid(core::UIDDomain::ROLLING_STOCK, core::UIDKind::VEHICLE, 0, 1);
constexpr core::UID kVeh2 =
    core::make_uid(core::UIDDomain::ROLLING_STOCK, core::UIDKind::VEHICLE, 0, 2);
constexpr core::UID kVtype1 =
    core::make_uid(core::UIDDomain::ROLLING_STOCK, core::UIDKind::VEHICLE_TYPE, 0, 1);
constexpr core::UID kSecA =
    core::make_uid(core::UIDDomain::INFRASTRUCTURE, core::UIDKind::TRACK_SECTION, 1, 1);
constexpr core::UID kSecB =
    core::make_uid(core::UIDDomain::INFRASTRUCTURE, core::UIDKind::TRACK_SECTION, 1, 2);

}  // namespace

TEST(TrainSim, MakeTrainSimStateAggregatesVehicles)
{
    core::TrainConsist consist{};
    consist.uid = kTrain1;
    consist.consist_lambda_pct = 100.0f;
    consist.vehicle_uids = {kVeh1, kVeh2};

    core::Vehicle v1{};
    v1.uid = kVeh1;
    v1.effective_mass_t = 80.0f;
    v1.max_speed_kmh = 120;
    v1.traction_force_kn = 280.0f;
    v1.davis = core::DavisCoefficients{39.24f, 0.1962f, 0.0017658f};

    core::Vehicle v2{};
    v2.uid = kVeh2;
    v2.effective_mass_t = 20.0f;
    v2.max_speed_kmh = 100;
    v2.traction_force_kn = std::nullopt;
    v2.davis = core::DavisCoefficients{14.715f, 0.07848f, 0.0007848f};

    const sim::TrainSimState state =
        sim::make_train_sim_state(consist, std::vector<core::Vehicle>{v1, v2}, kSecA);

    EXPECT_NEAR(state.physics_params.total_mass_t, 100.0f, 0.001f);
    EXPECT_NEAR(state.physics_params.max_traction_kn, 280.0f, 0.001f);
    EXPECT_NEAR(state.physics_params.max_speed_ms, 100.0f / 3.6f, 0.001f);
    EXPECT_GT(state.max_brake_kn, 0.0f);
}

TEST(TrainSim, MakeTrainSimStateCouplesSameTypeCapableLocomotives)
{
    core::TrainConsist consist{};
    consist.uid = kTrain2;
    consist.consist_lambda_pct = 100.0f;
    consist.vehicle_uids = {kVeh1, kVeh2};

    core::Vehicle v1{};
    v1.uid = kVeh1;
    v1.type_uid = kVtype1;
    v1.vehicle_type = "LOCOMOTIVE";
    v1.vehicle_subtype = "ELECTRIC";
    v1.effective_mass_t = 80.0f;
    v1.max_speed_kmh = 120;
    v1.traction_capable = true;
    v1.traction_status = core::TractionStatus::OPERATIONAL;
    v1.multiple_coupling_capable = true;
    v1.traction_force_kn = 280.0f;
    v1.davis = core::DavisCoefficients{39.24f, 0.1962f, 0.0017658f};

    core::Vehicle v2 = v1;
    v2.uid = kVeh2;

    const sim::TrainSimState state =
        sim::make_train_sim_state(consist, std::vector<core::Vehicle>{v1, v2}, kSecA);

    EXPECT_NEAR(state.physics_params.max_traction_kn, 560.0f, 0.001f);
}

TEST(TrainSim, MakeTrainSimStateKeepsUnknownCouplingAsBallast)
{
    core::TrainConsist consist{};
    consist.uid = kTrain3;
    consist.consist_lambda_pct = 100.0f;
    consist.vehicle_uids = {kVeh1, kVeh2};

    core::Vehicle v1{};
    v1.uid = kVeh1;
    v1.type_uid = kVtype1;
    v1.vehicle_type = "LOCOMOTIVE";
    v1.vehicle_subtype = "ELECTRIC";
    v1.effective_mass_t = 80.0f;
    v1.max_speed_kmh = 120;
    v1.traction_capable = true;
    v1.traction_status = core::TractionStatus::OPERATIONAL;
    v1.multiple_coupling_capable = std::nullopt;
    v1.traction_force_kn = 280.0f;
    v1.davis = core::DavisCoefficients{39.24f, 0.1962f, 0.0017658f};

    core::Vehicle v2 = v1;
    v2.uid = kVeh2;

    const sim::TrainSimState state =
        sim::make_train_sim_state(consist, std::vector<core::Vehicle>{v1, v2}, kSecA);

    EXPECT_NEAR(state.physics_params.max_traction_kn, 280.0f, 0.001f);
}

TEST(TrainSim, MakeTrainSimStateTreatsDefectiveEmuMotorAsBallast)
{
    core::TrainConsist consist{};
    consist.uid = kTrain4;
    consist.consist_lambda_pct = 100.0f;
    consist.vehicle_uids = {kVeh1};

    core::Vehicle motor{};
    motor.uid = kVeh1;
    motor.vehicle_type = "EMU_UNIT";
    motor.vehicle_subtype = "MOTOR";
    motor.effective_mass_t = 40.0f;
    motor.max_speed_kmh = 120;
    motor.traction_capable = true;
    motor.traction_status = core::TractionStatus::DEFECTIVE;
    motor.traction_force_kn = 150.0f;
    motor.davis = core::DavisCoefficients{34.335f, 0.17658f, 0.0014715f};

    const sim::TrainSimState state =
        sim::make_train_sim_state(consist, std::vector<core::Vehicle>{motor}, kSecA);

    EXPECT_NEAR(state.physics_params.max_traction_kn, 0.0f, 0.001f);
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
    initial.train_uid = kTrain1;
    initial.current_section_uid = kSecA;
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
    initial.train_uid = kTrain1;
    initial.current_section_uid = kSecA;
    initial.physics_params =
        physics::TrainPhysicsParams{100.0f, 300.0f, 30.0f, 1000.0f, 1.0f, 0.01f};
    initial.max_brake_kn = 120.0f;

    sim::TrainSim sim_train(initial, policy, integrator, sink);

    sim::TrainSimTickInput in{};
    in.driver_input = physics::DriverInput{core::SignalAspect::S2_PROCEED, 1000.0f, 30.0f, 120.0f,
                                           core::SignalAspect::S2_PROCEED, 1000.0f};
    in.section_length_m = 100.0f;
    in.next_section_uid = kSecB;

    const auto out = sim_train.tick(0.05f, in);

    ASSERT_TRUE(out.crossing.has_value());
    EXPECT_EQ(out.crossing->from_section_uid.value, kSecA.value);
    EXPECT_EQ(out.crossing->to_section_uid.value, kSecB.value);
    EXPECT_NEAR(out.crossing->overshoot_m, 5.0f, 0.001f);
    EXPECT_EQ(out.state.current_section_uid.value, kSecB.value);
    EXPECT_NEAR(out.state.physics_state.position_m, 5.0f, 0.001f);

    EXPECT_EQ(sink->crossing_count, 1);
    EXPECT_EQ(sink->last_train_uid.value, kTrain1.value);
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
    initial.train_uid = kTrain1;
    initial.current_section_uid = kSecA;
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
