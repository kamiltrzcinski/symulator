#include "engine/sim/train_sim.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace engine::sim
{

namespace
{

[[nodiscard]] std::string to_upper(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
    return s;
}

[[nodiscard]] bool is_locomotive(const core::Vehicle& vehicle)
{
    return to_upper(vehicle.vehicle_type) == "LOCOMOTIVE";
}

[[nodiscard]] bool is_operational_traction_unit(const core::Vehicle& vehicle)
{
    const bool traction_capable = vehicle.traction_capable ||
                                  vehicle.traction_force_kn.has_value() ||
                                  vehicle.power_kw.has_value();
    if (!traction_capable)
    {
        return false;
    }

    return !vehicle.traction_status.has_value() ||
           *vehicle.traction_status == core::TractionStatus::OPERATIONAL;
}

}  // namespace

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

void NullTrainEventSink::on_section_crossing(core::UID train_uid, const SectionCrossing& crossing)
{
    (void)train_uid;
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

        if (input.next_section_uid.has_value())
        {
            crossing =
                SectionCrossing{state_.current_section_uid, *input.next_section_uid, overshoot};
            state_.current_section_uid = *input.next_section_uid;
            state_.physics_state.position_m = overshoot;
            event_sink_->on_section_crossing(state_.train_uid, *crossing);
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
                                   core::UID initial_section_uid)
{
    if (vehicles.empty())
    {
        throw std::invalid_argument("make_train_sim_state requires at least one vehicle");
    }

    if (vehicles.size() != consist.vehicle_uids.size())
    {
        throw std::invalid_argument("Vehicle list size must match consist.vehicle_uids size");
    }

    std::vector<physics::VehiclePhysicsContrib> contrib;
    contrib.reserve(vehicles.size());
    std::vector<std::size_t> operational_locomotive_indices;

    for (const auto& vehicle : vehicles)
    {
        float traction_kn = 0.0f;
        if (is_operational_traction_unit(vehicle) && !is_locomotive(vehicle))
        {
            traction_kn = vehicle.traction_force_kn.value_or(0.0f);
        }

        if (is_operational_traction_unit(vehicle) && is_locomotive(vehicle))
        {
            operational_locomotive_indices.push_back(contrib.size());
        }

        contrib.push_back(physics::VehiclePhysicsContrib{
            .mass_t = vehicle.effective_mass_t,
            .traction_kn = traction_kn,
            .max_speed_ms = static_cast<float>(vehicle.max_speed_kmh) / 3.6f,
            .davis_a = vehicle.davis.a,
            .davis_b = vehicle.davis.b,
            .davis_c = vehicle.davis.c,
        });
    }

    if (!operational_locomotive_indices.empty())
    {
        if (operational_locomotive_indices.size() == 1)
        {
            const std::size_t idx = operational_locomotive_indices.front();
            contrib[idx].traction_kn = vehicles[idx].traction_force_kn.value_or(0.0f);
        }
        else
        {
            const std::size_t first_idx = operational_locomotive_indices.front();
            const core::Vehicle& first_locomotive = vehicles[first_idx];
            const bool same_type = std::all_of(
                operational_locomotive_indices.begin(), operational_locomotive_indices.end(),
                [&vehicles, &first_locomotive](std::size_t idx)
                { return vehicles[idx].type_uid == first_locomotive.type_uid; });

            const bool coupling_allowed =
                same_type && first_locomotive.multiple_coupling_capable.value_or(false);

            if (coupling_allowed)
            {
                for (const std::size_t idx : operational_locomotive_indices)
                {
                    contrib[idx].traction_kn = vehicles[idx].traction_force_kn.value_or(0.0f);
                }
            }
            else
            {
                // Unknown/unsupported/mixed coupling: first operational locomotive provides traction,
                // additional locomotives remain ballast.
                contrib[first_idx].traction_kn = first_locomotive.traction_force_kn.value_or(0.0f);
            }
        }
    }

    TrainSimState state{};
    state.train_uid = consist.uid;
    state.current_section_uid = initial_section_uid;
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
