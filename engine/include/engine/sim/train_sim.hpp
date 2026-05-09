#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "engine/core/fleet_registry.hpp"
#include "engine/physics/driver_ai.hpp"
#include "engine/physics/physics_model.hpp"

namespace engine::sim
{

struct SectionCrossing
{
    core::GID from_section_gid;
    core::GID to_section_gid;
    float overshoot_m = 0.0f;
};

struct TrainSimState
{
    core::GID train_gid;
    core::GID current_section_gid;

    physics::TrainPhysicsParams physics_params{};
    physics::TrainPhysicsState physics_state{};
    physics::DriverState driver_state = physics::DriverState::STOPPED;

    float max_brake_kn = 0.0f;
};

struct TrainSimTickInput
{
    physics::DriverInput driver_input;
    float section_length_m = 0.0f;
    std::optional<core::GID> next_section_gid;
};

struct TrainSimOutput
{
    TrainSimState state;
    std::optional<SectionCrossing> crossing;
};

class ITrainControlPolicy
{
public:
    virtual ~ITrainControlPolicy() = default;

    virtual physics::DriverOutput compute(physics::DriverState prev_state,
                                          const physics::TrainPhysicsParams& params,
                                          const physics::TrainPhysicsState& state,
                                          const physics::DriverInput& input) const = 0;
};

class IPhysicsIntegrator
{
public:
    virtual ~IPhysicsIntegrator() = default;

    virtual physics::TrainPhysicsState step(const physics::TrainPhysicsParams& params,
                                            const physics::TrainPhysicsState& state,
                                            float dt_s) const = 0;
};

class ITrainEventSink
{
public:
    virtual ~ITrainEventSink() = default;

    virtual void on_section_crossing(const core::GID& train_gid,
                                     const SectionCrossing& crossing) = 0;
};

class DriverAIPolicy final : public ITrainControlPolicy
{
public:
    physics::DriverOutput compute(physics::DriverState prev_state,
                                  const physics::TrainPhysicsParams& params,
                                  const physics::TrainPhysicsState& state,
                                  const physics::DriverInput& input) const override;
};

class PhysicsModelIntegrator final : public IPhysicsIntegrator
{
public:
    physics::TrainPhysicsState step(const physics::TrainPhysicsParams& params,
                                    const physics::TrainPhysicsState& state,
                                    float dt_s) const override;
};

class NullTrainEventSink final : public ITrainEventSink
{
public:
    void on_section_crossing(const core::GID& train_gid, const SectionCrossing& crossing) override;
};

class TrainSim
{
public:
    explicit TrainSim(TrainSimState initial_state,
                      std::shared_ptr<ITrainControlPolicy> control_policy = nullptr,
                      std::shared_ptr<IPhysicsIntegrator> integrator = nullptr,
                      std::shared_ptr<ITrainEventSink> event_sink = nullptr);

    TrainSimOutput tick(float dt_s, const TrainSimTickInput& input);

    const TrainSimState& state() const noexcept { return state_; }

private:
    TrainSimState state_;
    std::shared_ptr<ITrainControlPolicy> control_policy_;
    std::shared_ptr<IPhysicsIntegrator> integrator_;
    std::shared_ptr<ITrainEventSink> event_sink_;
};

TrainSimState make_train_sim_state(const core::TrainConsist& consist,
                                   const std::vector<core::Vehicle>& vehicles,
                                   const core::GID& initial_section_gid);

}  // namespace engine::sim
