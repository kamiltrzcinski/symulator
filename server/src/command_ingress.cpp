// server/src/command_ingress.cpp

#include "server/command_ingress.hpp"

#include "commands_generated.h"
#include "common_generated.h"

#include <flatbuffers/verifier.h>

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
    // Both enums have identical ordinal layout (verified by static_assert in types.hpp).
    return static_cast<engine::core::SignalAspect>(a);
}

static engine::core::DerailerState from_proto(proto::DerailerPosition p)
{
    using DS = engine::core::DerailerState;
    return (p == proto::DerailerPosition_UNLOCKED) ? DS::UNLOCKED : DS::LOCKED;
}

static engine::core::BlockSectionState from_proto(proto::BlockSectionState s)
{
    // Proto: OPEN=0, CLOSED=1.  Engine: CLOSED=0, OPEN=1 — inverted.
    using BS = engine::core::BlockSectionState;
    return (s == proto::BlockSectionState_OPEN) ? BS::OPEN : BS::CLOSED;
}

static engine::core::Shl12Op from_proto(proto::Shl12Op op)
{
    // Ordinals match.
    return static_cast<engine::core::Shl12Op>(op);
}

// ── parse_command ─────────────────────────────────────────────────────────────

std::optional<engine::core::Command> CommandIngress::parse_command(uint8_t cmd_type,
                                                                   const uint8_t* fb_data,
                                                                   uint32_t fb_size)
{
    using namespace engine::core;

    // Verify FlatBuffers buffer is minimally sane.
    flatbuffers::Verifier verifier(fb_data, static_cast<std::size_t>(fb_size));

    switch (cmd_type)
    {
        case cmd::kSetSwitchPosition:
        {
            if (!verifier.VerifyBuffer<proto::SetSwitchPosition>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::SetSwitchPosition>(fb_data);
            if (!msg->g_id())
                return std::nullopt;
            return SetSwitchPositionCmd{GID{msg->g_id()->str()}, from_proto(msg->position())};
        }

        case cmd::kSetSignalAspect:
        {
            if (!verifier.VerifyBuffer<proto::SetSignalAspect>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::SetSignalAspect>(fb_data);
            if (!msg->g_id())
                return std::nullopt;
            return SetSignalAspectCmd{GID{msg->g_id()->str()}, from_proto(msg->aspect())};
        }

        case cmd::kSetDerailerPosition:
        {
            if (!verifier.VerifyBuffer<proto::SetDerailerPosition>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::SetDerailerPosition>(fb_data);
            if (!msg->g_id())
                return std::nullopt;
            return SetDerailerPositionCmd{GID{msg->g_id()->str()}, from_proto(msg->position())};
        }

        case cmd::kSetBlockSection:
        {
            if (!verifier.VerifyBuffer<proto::SetBlockSection>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::SetBlockSection>(fb_data);
            if (!msg->g_id())
                return std::nullopt;
            return SetBlockSectionCmd{GID{msg->g_id()->str()}, from_proto(msg->state())};
        }

        case cmd::kRequestRoute:
        {
            if (!verifier.VerifyBuffer<proto::RequestRoute>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::RequestRoute>(fb_data);
            if (!msg->from_signal_g_id() || !msg->to_signal_g_id())
                return std::nullopt;
            return RequestRouteCmd{GID{msg->from_signal_g_id()->str()},
                                   GID{msg->to_signal_g_id()->str()}};
        }

        case cmd::kCancelRoute:
        {
            if (!verifier.VerifyBuffer<proto::CancelRoute>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::CancelRoute>(fb_data);
            if (!msg->route_id())
                return std::nullopt;
            return CancelRouteCmd{GID{msg->route_id()->str()}, false};
        }

        case cmd::kAcknowledgeAlarm:
        {
            if (!verifier.VerifyBuffer<proto::AcknowledgeAlarm>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::AcknowledgeAlarm>(fb_data);
            if (!msg->alarm_id())
                return std::nullopt;
            return AcknowledgeAlarmCmd{GID{msg->alarm_id()->str()}};
        }

        case cmd::kSetBlockDirection:
        {
            if (!verifier.VerifyBuffer<proto::SetBlockDirection>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::SetBlockDirection>(fb_data);
            if (!msg->block_section_g_id())
                return std::nullopt;
            return SetBlockDirectionCmd{GID{msg->block_section_g_id()->str()},
                                        from_proto(msg->operation())};
        }

        case cmd::kInitAxleCounterReset:
        {
            if (!verifier.VerifyBuffer<proto::InitAxleCounterReset>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::InitAxleCounterReset>(fb_data);
            if (!msg->block_section_g_id())
                return std::nullopt;
            return InitAxleCounterResetCmd{GID{msg->block_section_g_id()->str()}};
        }

        case cmd::kResetAxleCounter:
        {
            if (!verifier.VerifyBuffer<proto::ResetAxleCounter>())
                return std::nullopt;
            const auto* msg = flatbuffers::GetRoot<proto::ResetAxleCounter>(fb_data);
            if (!msg->block_section_g_id())
                return std::nullopt;
            return ResetAxleCounterCmd{GID{msg->block_section_g_id()->str()}};
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
