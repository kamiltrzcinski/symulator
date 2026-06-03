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

    // SetSwitchPosition now uses uint64 uid, not string g_id.
    proto::SetSwitchPositionBuilder cmd(builder);
    cmd.add_uid(42001ULL);
    cmd.add_position(proto::SwitchPosition_STRAIGHT);

    const auto payload = cmd.Finish();
    builder.Finish(payload);

    const auto* parsed = flatbuffers::GetRoot<proto::SetSwitchPosition>(builder.GetBufferPointer());
    ASSERT_NE(parsed, nullptr);
    EXPECT_EQ(parsed->uid(), 42001ULL);
    EXPECT_EQ(parsed->position(), proto::SwitchPosition_STRAIGHT);
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
        // TakeoverRequest now uses uint64 dispatch_area_uid and station_uid.
        proto::TakeoverRequestBuilder takeover(builder);
        takeover.add_dispatch_area_uid(55001ULL);
        takeover.add_station_uid(22001ULL);

        const auto payload = takeover.Finish();
        builder.Finish(payload);

        const auto* parsed =
            flatbuffers::GetRoot<proto::TakeoverRequest>(builder.GetBufferPointer());
        ASSERT_NE(parsed, nullptr);
        EXPECT_EQ(parsed->dispatch_area_uid(), 55001ULL);
        EXPECT_EQ(parsed->station_uid(), 22001ULL);
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
    EXPECT_EQ(static_cast<std::uint8_t>(proto::SwitchPosition_STRAIGHT), 0);
    EXPECT_EQ(static_cast<std::uint8_t>(proto::Aspect_S1_STOP), 0);
}
