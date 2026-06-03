// server/src/dispatch_channel.cpp

#include "server/dispatch_channel.hpp"
#include "server/frame.hpp"
#include "server/transport_gateway.hpp"

#include "common_generated.h"
#include "dispatch_channel_generated.h"

#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/verifier.h>

#include <chrono>
#include <string>

namespace server
{

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::uint64_t now_us() noexcept
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<microseconds>(system_clock::now().time_since_epoch()).count());
}

static engine::core::DispatchFormType to_engine_form(proto::DispatchFormType ft) noexcept
{
    switch (ft)
    {
        case proto::DispatchFormType_S2:
            return engine::core::DispatchFormType::S2;
        case proto::DispatchFormType_S24:
            return engine::core::DispatchFormType::S24;
        case proto::DispatchFormType_S25:
            return engine::core::DispatchFormType::S25;
        case proto::DispatchFormType_S26:
            return engine::core::DispatchFormType::S26;
        case proto::DispatchFormType_S55:
            return engine::core::DispatchFormType::S55;
        case proto::DispatchFormType_S56:
            return engine::core::DispatchFormType::S56;
    }
    return engine::core::DispatchFormType::S2;
}

static proto::ExchangeStatus to_proto_status(engine::core::ExchangeStatus s) noexcept
{
    switch (s)
    {
        case engine::core::ExchangeStatus::IDLE:
        case engine::core::ExchangeStatus::S2_SENT:
        case engine::core::ExchangeStatus::S25_SENT:
            return proto::ExchangeStatus_PENDING;
        case engine::core::ExchangeStatus::S24_RECEIVED:
        case engine::core::ExchangeStatus::S26_RECEIVED:
            return proto::ExchangeStatus_ACCEPTED;
        case engine::core::ExchangeStatus::CANCELLED:
            return proto::ExchangeStatus_REJECTED;
        case engine::core::ExchangeStatus::CLOSED:
            return proto::ExchangeStatus_CLOSED;
    }
    return proto::ExchangeStatus_PENDING;
}

// ── DispatchChannel ───────────────────────────────────────────────────────────

DispatchChannel::DispatchChannel(DispatchCoordinator& coordinator, TransportGateway& gateway)
    : coordinator_{coordinator}, gateway_{gateway}
{
}

void DispatchChannel::on_inbound(const std::vector<std::uint8_t>& payload,
                                 const std::string& sender_client_id,
                                 const std::string& sender_area_id)
{
    flatbuffers::Verifier verifier(payload.data(), payload.size());
    if (!verifier.VerifyBuffer<proto::DispatchChannelMessage>())
        return;

    const auto* msg = flatbuffers::GetRoot<proto::DispatchChannelMessage>(payload.data());

    // Area UIDs from wire (uint64). Convert to strings for internal routing.
    const std::uint64_t src_uid = msg->src_area_uid();
    const std::uint64_t dst_uid = msg->dst_area_uid();
    if (src_uid == 0 || dst_uid == 0)
        return;

    const std::string src_area = std::to_string(src_uid);
    const std::string dst_area = std::to_string(dst_uid);

    if (src_area != sender_area_id)
        return;

    const std::uint64_t ts = now_us();

    if (msg->kind() == proto::DispatchChannelMessageKind_FREE_TEXT)
    {
        const auto* ft_payload = msg->body_as_FreeTextPayload();
        if (!ft_payload || !ft_payload->body())
            return;

        const std::string body = ft_payload->body()->str();
        coordinator_.handle_free_text(src_area, dst_area, body, ts);

        flatbuffers::FlatBufferBuilder fbb(512);
        auto sender_off = fbb.CreateString(sender_client_id);
        auto body_str = fbb.CreateString(body);
        auto ft_off = proto::CreateFreeTextPayload(fbb, body_str);
        auto root = proto::CreateDispatchChannelMessage(
            fbb, src_uid, dst_uid, proto::TelegramDirection_SENT,
            proto::DispatchChannelMessageKind_FREE_TEXT,
            proto::DispatchChannelMessageBody_FreeTextPayload, ft_off.Union(),
            /*exchange_uid=*/0, proto::ExchangeStatus_ACCEPTED, ts, sender_off);
        fbb.Finish(root);

        auto wire = encode_frame(msg_type::kDispatchChannel, 0, 0,
                                 {fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize()});
        gateway_.broadcast_to_pair(src_area, dst_area, std::move(wire));
        return;
    }

    const auto* df_payload = msg->body_as_DispatchFormPayload();
    if (!df_payload || !df_payload->train_number())
        return;

    const std::string train_number = df_payload->train_number()->str();
    const engine::core::DispatchFormType engine_form = to_engine_form(df_payload->form_type());
    const engine::core::TelegramDirection direction =
        (msg->direction() == proto::TelegramDirection_SENT)
            ? engine::core::TelegramDirection::SENT
            : engine::core::TelegramDirection::RECEIVED;

    std::optional<std::string> track_number;
    if (df_payload->track_number())
        track_number = df_payload->track_number()->str();

    std::vector<std::string> km_markers;
    if (const auto* km = df_payload->km_markers())
    {
        km_markers.reserve(km->size());
        for (const auto* s : *km)
            if (s)
                km_markers.push_back(s->str());
    }

    const auto outcome = coordinator_.handle_dispatch_form(
        engine_form, direction, src_area, dst_area, train_number, track_number, km_markers, ts);
    if (!outcome)
        return;

    // Parse exchange_id string back to uint64 for wire protocol.
    std::uint64_t exchange_uid = 0;
    try
    {
        exchange_uid = std::stoull(outcome->exchange_id);
    }
    catch (...)
    {
    }

    flatbuffers::FlatBufferBuilder fbb(512);
    auto sender_off = fbb.CreateString(sender_client_id);
    auto train_off = fbb.CreateString(train_number);

    std::vector<flatbuffers::Offset<flatbuffers::String>> km_offs;
    for (const auto& km : km_markers)
        km_offs.push_back(fbb.CreateString(km));
    auto km_vec = fbb.CreateVector(km_offs);

    flatbuffers::Offset<flatbuffers::String> track_off = 0;
    if (df_payload->track_number())
        track_off = fbb.CreateString(df_payload->track_number()->str());

    auto dfp_off = proto::CreateDispatchFormPayload(fbb, df_payload->form_type(), train_off, km_vec,
                                                    track_off);

    auto root = proto::CreateDispatchChannelMessage(
        fbb, src_uid, dst_uid, msg->direction(), proto::DispatchChannelMessageKind_DISPATCH_FORM,
        proto::DispatchChannelMessageBody_DispatchFormPayload, dfp_off.Union(), exchange_uid,
        to_proto_status(outcome->new_status), ts, sender_off);
    fbb.Finish(root);

    auto wire = encode_frame(msg_type::kDispatchChannel, 0, 0,
                             {fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize()});
    gateway_.broadcast_to_pair(src_area, dst_area, std::move(wire));
}

}  // namespace server
