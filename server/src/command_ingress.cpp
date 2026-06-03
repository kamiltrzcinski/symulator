// server/src/command_ingress.cpp

#include "server/command_ingress.hpp"

#include "commands_generated.h"
#include "common_generated.h"

#include <flatbuffers/verifier.h>

#include <cstdint>

namespace server
{

// ── cmd_type constants (from doc 09) ─────────────────────────────────────────
namespace cmd
{
constexpr uint8_t kSetSwitchPosition = 0x01;
constexpr uint8_t kSetSignalAspect = 0x02;
constexpr uint8_t kSetDerailerPosition = 0x03;
constexpr uint8_t kSetBlockSection = 0x04;
constexpr uint8_t kRequestRoute = 0x05;
constexpr uint8_t kCancelRoute = 0x06;
constexpr uint8_t kAcknowledgeAlarm = 0x07;
constexpr uint8_t kSetBlockDirection = 0x08;
constexpr uint8_t kInitAxleCounterReset = 0x09;
constexpr uint8_t kResetAxleCounter = 0x0A;
constexpr uint8_t kOperatorCommand = 0x20;
constexpr uint8_t kMl8Command = 0x21;
}  // namespace cmd

// ── Proto → Engine enum conversions ──────────────────────────────────────────

static engine::core::SwitchPosition from_proto(proto::SwitchPosition p)
{
    using SP = engine::core::SwitchPosition;
    switch (p)
    {
        case proto::SwitchPosition_STRAIGHT:
            return SP::STRAIGHT;
        case proto::SwitchPosition_DIVERGENT:
            return SP::DIVERGENT;
        default:
            return SP::STRAIGHT;
    }
}

static engine::core::SignalAspect from_proto(proto::Aspect a)
{
    return static_cast<engine::core::SignalAspect>(a);
}

static engine::core::DerailerState from_proto(proto::DerailerPosition p)
{
    return (p == proto::DerailerPosition_UNLOCKED) ? engine::core::DerailerState::UNLOCKED
                                                   : engine::core::DerailerState::LOCKED;
}

static engine::core::BlockSectionState from_proto(proto::BlockSectionState s)
{
    using BS = engine::core::BlockSectionState;
    return (s == proto::BlockSectionState_OPEN) ? BS::OPEN : BS::CLOSED;
}

static engine::core::Shl12Op from_proto(proto::Shl12Op op)
{
    return static_cast<engine::core::Shl12Op>(op);
}

static engine::core::OperatorTargetKind from_proto(proto::OperatorTargetKind kind)
{
    return static_cast<engine::core::OperatorTargetKind>(kind);
}

static engine::core::OperatorCommandCode from_proto(proto::OperatorCommandCode code)
{
    return static_cast<engine::core::OperatorCommandCode>(code);
}

static engine::core::Ml8CommandCode from_proto(proto::Ml8CommandCode code)
{
    return static_cast<engine::core::Ml8CommandCode>(code);
}

// ── parse_command ─────────────────────────────────────────────────────────────

std::optional<engine::core::Command> CommandIngress::parse_command(uint8_t cmd_type,
                                                                   const uint8_t* fb_data,
                                                                   uint32_t fb_size)
{
    using namespace engine::core;

    flatbuffers::Verifier verifier(fb_data, static_cast<std::size_t>(fb_size));

    switch (cmd_type)
    {
        case cmd::kSetSwitchPosition:
        {
            if (!verifier.VerifyBuffer<proto::SetSwitchPosition>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::SetSwitchPosition>(fb_data);
            if (msg->uid() == 0)
                return std::nullopt;
            return SetSwitchPositionCmd{UID{msg->uid()}, from_proto(msg->position())};
        }

        case cmd::kSetSignalAspect:
        {
            if (!verifier.VerifyBuffer<proto::SetSignalAspect>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::SetSignalAspect>(fb_data);
            if (msg->uid() == 0)
                return std::nullopt;
            return SetSignalAspectCmd{UID{msg->uid()}, from_proto(msg->aspect())};
        }

        case cmd::kSetDerailerPosition:
        {
            if (!verifier.VerifyBuffer<proto::SetDerailerPosition>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::SetDerailerPosition>(fb_data);
            if (msg->uid() == 0)
                return std::nullopt;
            return SetDerailerPositionCmd{UID{msg->uid()}, from_proto(msg->position())};
        }

        case cmd::kSetBlockSection:
        {
            if (!verifier.VerifyBuffer<proto::SetBlockSection>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::SetBlockSection>(fb_data);
            if (msg->uid() == 0)
                return std::nullopt;
            return SetBlockSectionCmd{UID{msg->uid()}, from_proto(msg->state())};
        }

        case cmd::kRequestRoute:
        {
            if (!verifier.VerifyBuffer<proto::RequestRoute>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::RequestRoute>(fb_data);
            if (msg->from_signal_uid() == 0 || msg->to_signal_uid() == 0)
                return std::nullopt;
            return RequestRouteCmd{UID{msg->from_signal_uid()}, UID{msg->to_signal_uid()}};
        }

        case cmd::kCancelRoute:
        {
            if (!verifier.VerifyBuffer<proto::CancelRoute>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::CancelRoute>(fb_data);
            if (msg->route_uid() == 0)
                return std::nullopt;
            return CancelRouteCmd{UID{msg->route_uid()}, false};
        }

        case cmd::kAcknowledgeAlarm:
        {
            if (!verifier.VerifyBuffer<proto::AcknowledgeAlarm>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::AcknowledgeAlarm>(fb_data);
            if (msg->alarm_uid() == 0)
                return std::nullopt;
            return AcknowledgeAlarmCmd{UID{msg->alarm_uid()}};
        }

        case cmd::kSetBlockDirection:
        {
            if (!verifier.VerifyBuffer<proto::SetBlockDirection>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::SetBlockDirection>(fb_data);
            if (msg->block_section_uid() == 0)
                return std::nullopt;
            return SetBlockDirectionCmd{UID{msg->block_section_uid()},
                                        from_proto(msg->operation())};
        }

        case cmd::kInitAxleCounterReset:
        {
            if (!verifier.VerifyBuffer<proto::InitAxleCounterReset>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::InitAxleCounterReset>(fb_data);
            if (msg->block_section_uid() == 0)
                return std::nullopt;
            return InitAxleCounterResetCmd{UID{msg->block_section_uid()}};
        }

        case cmd::kResetAxleCounter:
        {
            if (!verifier.VerifyBuffer<proto::ResetAxleCounter>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::ResetAxleCounter>(fb_data);
            if (msg->block_section_uid() == 0)
                return std::nullopt;
            return ResetAxleCounterCmd{UID{msg->block_section_uid()}};
        }

        case cmd::kOperatorCommand:
        {
            if (!verifier.VerifyBuffer<proto::OperatorCommand>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::OperatorCommand>(fb_data);
            if (msg->target_uid() == 0)
                return std::nullopt;
            return OperatorCommandCmd{UID{msg->target_uid()}, from_proto(msg->target_kind()),
                                      from_proto(msg->command_code())};
        }

        case cmd::kMl8Command:
        {
            if (!verifier.VerifyBuffer<proto::Ml8Command>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::Ml8Command>(fb_data);
            if (msg->target_uid() == 0)
                return std::nullopt;
            return Ml8CommandCmd{UID{msg->target_uid()}, from_proto(msg->target_kind()),
                                 from_proto(msg->command_code())};
        }

        default:
            return std::nullopt;
    }
}

std::optional<engine::core::Command> CommandIngress::parse_payload(const uint8_t* payload,
                                                                   uint32_t payload_len)
{
    if (!payload || payload_len < 1)
        return std::nullopt;
    const uint8_t cmd_type = payload[0];
    return parse_command(cmd_type, payload + 1, payload_len - 1);
}

}  // namespace server
