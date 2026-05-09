#include <cstdint>

#include <gtest/gtest.h>

#include "chat_generated.h"
#include "commands_generated.h"
#include "common_generated.h"
#include "events_generated.h"
#include "ownership_generated.h"
#include "session_generated.h"
#include "snapshot_generated.h"
#include "voice_generated.h"

TEST(ProtoSchemas, CommandPayloadRoundTrip)
{
    flatbuffers::FlatBufferBuilder builder;

    const auto gid = builder.CreateString("SW-1");
    proto::SetSwitchPositionBuilder cmd(builder);
    cmd.add_g_id(gid);
    cmd.add_position(proto::SwitchPosition_NORMAL);

    const auto payload = cmd.Finish();
    builder.Finish(payload);

    const auto* parsed = flatbuffers::GetRoot<proto::SetSwitchPosition>(builder.GetBufferPointer());
    ASSERT_NE(parsed, nullptr);
    ASSERT_NE(parsed->g_id(), nullptr);
    EXPECT_EQ(parsed->g_id()->str(), "SW-1");
    EXPECT_EQ(parsed->position(), proto::SwitchPosition_NORMAL);
}

TEST(ProtoSchemas, LifecycleOwnershipChatVoiceAndSnapshotPayloadsBuild)
{
    flatbuffers::FlatBufferBuilder builder;

    {
        const auto message = builder.CreateString("maintenance window");
        proto::SessionNoticeBuilder notice(builder);
        notice.add_type(proto::SessionNoticeType_WARNING);
        notice.add_message(message);

        const auto payload = notice.Finish();
        builder.Finish(payload);

        const auto* parsed = flatbuffers::GetRoot<proto::SessionNotice>(builder.GetBufferPointer());
        ASSERT_NE(parsed, nullptr);
        EXPECT_EQ(parsed->type(), proto::SessionNoticeType_WARNING);
    }

    builder.Clear();
    {
        const auto posterunek = builder.CreateString("GGO_A");
        const auto station = builder.CreateString("GGO");
        proto::TakeoverRequestBuilder takeover(builder);
        takeover.add_posterunek_id(posterunek);
        takeover.add_station_sid(station);

        const auto payload = takeover.Finish();
        builder.Finish(payload);

        const auto* parsed =
            flatbuffers::GetRoot<proto::TakeoverRequest>(builder.GetBufferPointer());
        ASSERT_NE(parsed, nullptr);
        ASSERT_NE(parsed->posterunek_id(), nullptr);
        EXPECT_EQ(parsed->posterunek_id()->str(), "GGO_A");
    }

    builder.Clear();
    {
        const auto text = builder.CreateString("test message");
        proto::ChatMessageBuilder chat(builder);
        chat.add_target(proto::ChatTarget_BROADCAST);
        chat.add_text(text);

        const auto payload = chat.Finish();
        builder.Finish(payload);

        const auto* parsed = proto::GetChatMessage(builder.GetBufferPointer());
        ASSERT_NE(parsed, nullptr);
        EXPECT_EQ(parsed->target(), proto::ChatTarget_BROADCAST);
    }

    builder.Clear();
    {
        const auto channel = builder.CreateString("ops-main");
        proto::VoiceChanJoinBuilder join(builder);
        join.add_channel_id(channel);

        const auto payload = join.Finish();
        builder.Finish(payload);

        const auto* parsed = flatbuffers::GetRoot<proto::VoiceChanJoin>(builder.GetBufferPointer());
        ASSERT_NE(parsed, nullptr);
        ASSERT_NE(parsed->channel_id(), nullptr);
        EXPECT_EQ(parsed->channel_id()->str(), "ops-main");
    }

    builder.Clear();
    {
        const auto session = builder.CreateString("session-1");
        proto::SnapshotBuilder snapshot(builder);
        snapshot.add_schema_version(1);
        snapshot.add_session_id(session);
        snapshot.add_seq_cursor(0);
        snapshot.add_timestamp_us(1);

        const auto payload = snapshot.Finish();
        builder.Finish(payload);

        const auto* parsed = proto::GetSnapshot(builder.GetBufferPointer());
        ASSERT_NE(parsed, nullptr);
        ASSERT_NE(parsed->session_id(), nullptr);
        EXPECT_EQ(parsed->session_id()->str(), "session-1");
    }
}

TEST(ProtoSchemas, CriticalEnumDefaultsAreStable)
{
    EXPECT_EQ(static_cast<std::uint8_t>(proto::NakReason_UNSPECIFIED), 0);
    EXPECT_EQ(static_cast<std::uint8_t>(proto::ChatTarget_BROADCAST), 0);
    EXPECT_EQ(static_cast<std::uint8_t>(proto::SessionNoticeType_INFO), 0);
    EXPECT_EQ(static_cast<std::uint8_t>(proto::SwitchPosition_NORMAL), 0);
    EXPECT_EQ(static_cast<std::uint8_t>(proto::AlarmType_TRACK_OCCUPIED_UNEXPECTEDLY), 0);
}
