#include "engine/sim/train_sim.hpp"

#include <algorithm>
#include <stdexcept>

namespace engine::sim
{

physics::DriverOutput DriverAIPolicy::compute(physics::DriverState prev_state,
                                              const physics::TrainPhysicsParams& params,
                                              const physics::TrainPhysicsState& state,
                                              const physics::DriverInput& input) const
{
    return physics::DriverAI::tick(prev_state, params, state, input);
}

physics::TrainPhysicsState PhysicsModelIntegrator::step(const physics::TrainPhysicsParams& params,
                                                        const physics::TrainPhysicsState& state,
                                                        float dt_s) const
{
    return physics::PhysicsModel::integrate(params, state, dt_s);
}

void NullTrainEventSink::on_section_crossing(const core::GID& train_gid,
                                             const SectionCrossing& crossing)
{
    (void)train_gid;
    (void)crossing;
}

TrainSim::TrainSim(TrainSimState initial_state, std::shared_ptr<ITrainControlPolicy> control_policy,
                   std::shared_ptr<IPhysicsIntegrator> integrator,
                   std::shared_ptr<ITrainEventSink> event_sink)
    : state_(std::move(initial_state)),
      control_policy_(control_policy ? std::move(control_policy)
                                     : std::make_shared<DriverAIPolicy>()),
      integrator_(integrator ? std::move(integrator) : std::make_shared<PhysicsModelIntegrator>()),
      event_sink_(event_sink ? std::move(event_sink) : std::make_shared<NullTrainEventSink>())
{
}

TrainSimOutput TrainSim::tick(float dt_s, const TrainSimTickInput& input)
{
    physics::DriverInput driver_input = input.driver_input;
    if (driver_input.max_brake_kn <= 0.0f)
    {
        driver_input.max_brake_kn = state_.max_brake_kn;
    }

    const physics::DriverOutput control = control_policy_->compute(
        state_.driver_state, state_.physics_params, state_.physics_state, driver_input);

    state_.physics_state.traction_kn = control.traction_kn;
    state_.physics_state.brake_kn = control.brake_kn;
    state_.physics_state = integrator_->step(state_.physics_params, state_.physics_state, dt_s);
    state_.driver_state = control.state;

    std::optional<SectionCrossing> crossing;
    if (input.section_length_m > 0.0f && state_.physics_state.position_m >= input.section_length_m)
    {
        const float overshoot = state_.physics_state.position_m - input.section_length_m;

        if (input.next_section_gid.has_value())
        {
            crossing =
                SectionCrossing{state_.current_section_gid, *input.next_section_gid, overshoot};
            state_.current_section_gid = *input.next_section_gid;
            state_.physics_state.position_m = overshoot;
            event_sink_->on_section_crossing(state_.train_gid, *crossing);
        }
        else
        {
            // Dead-end fallback: stop train at section boundary.
            state_.physics_state.position_m = input.section_length_m;
            state_.physics_state.velocity_ms = 0.0f;
            state_.physics_state.accel_ms2 = 0.0f;
            state_.physics_state.traction_kn = 0.0f;
            state_.physics_state.brake_kn = state_.max_brake_kn;
            state_.driver_state = physics::DriverState::STOPPED;
        }
    }

    return TrainSimOutput{state_, crossing};
}

TrainSimState make_train_sim_state(const core::TrainConsist& consist,
                                   const std::vector<core::Vehicle>& vehicles,
                                   const core::GID& initial_section_gid)
{
    if (vehicles.empty())
    {
        throw std::invalid_argument("make_train_sim_state requires at least one vehicle");
    }

    if (vehicles.size() != consist.vehicle_gids.size())
    {
        throw std::invalid_argument("Vehicle list size must match consist.vehicle_gids size");
    }

    std::vector<physics::VehiclePhysicsContrib> contrib;
    contrib.reserve(vehicles.size());
    for (const auto& vehicle : vehicles)
    {
        contrib.push_back(physics::VehiclePhysicsContrib{
            .mass_t = vehicle.effective_mass_t,
            .traction_kn = vehicle.traction_force_kn.value_or(0.0f),
            .max_speed_ms = static_cast<float>(vehicle.max_speed_kmh) / 3.6f,
            .davis_a = vehicle.davis.a,
            .davis_b = vehicle.davis.b,
            .davis_c = vehicle.davis.c,
        });
    }

    TrainSimState state{};
    state.train_gid = consist.gid;
    state.current_section_gid = initial_section_gid;
    state.physics_params = physics::build_train_params(contrib.data(), contrib.size());
    state.max_brake_kn =
        physics::PhysicsModel::max_brake_kn(state.physics_params, consist.consist_lambda_pct);

    state.physics_state = physics::TrainPhysicsState{};
    state.physics_state.traction_kn = 0.0f;
    state.physics_state.brake_kn = 0.0f;
    state.driver_state = physics::DriverState::STOPPED;

    return state;
}

}  // namespace engine::sim
