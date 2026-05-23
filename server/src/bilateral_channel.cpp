// server/src/bilateral_channel.cpp

#include "server/bilateral_channel.hpp"
#include "server/frame.hpp"
#include "server/transport_gateway.hpp"

#include "bilateral_generated.h"
#include "common_generated.h"

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

/// Map proto::DispatchFormType (FlatBuffers, matches common.fbs) to engine enum.
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

static std::string form_type_str(proto::DispatchFormType ft) noexcept
{
    switch (ft)
    {
        case proto::DispatchFormType_S2:
            return "S2";
        case proto::DispatchFormType_S24:
            return "S24";
        case proto::DispatchFormType_S25:
            return "S25";
        case proto::DispatchFormType_S26:
            return "S26";
        case proto::DispatchFormType_S55:
            return "S55";
        case proto::DispatchFormType_S56:
            return "S56";
    }
    return "UNKNOWN";
}

/// Map engine ExchangeStatus to the simplified proto ExchangeStatus.
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

// ── BilateralChannel ──────────────────────────────────────────────────────────

BilateralChannel::BilateralChannel(DispatchExchangeManager& exchanges, IDbWriter& db_writer,
                                   TransportGateway& gateway, std::string session_id)
    : exchanges_{exchanges},
      db_writer_{db_writer},
      gateway_{gateway},
      session_id_{std::move(session_id)}
{
}

void BilateralChannel::on_inbound(const std::vector<std::uint8_t>& payload,
                                  const std::string& sender_client_id,
                                  const std::string& sender_area_id)
{
    // 1. Verify FlatBuffers payload
    flatbuffers::Verifier verifier(payload.data(), payload.size());
    if (!verifier.VerifyBuffer<proto::BilateralMessage>())
        return;  // malformed — drop silently

    const auto* msg = flatbuffers::GetRoot<proto::BilateralMessage>(payload.data());

    if (!msg->src_area_id() || !msg->dst_area_id())
        return;

    const std::string src_area = msg->src_area_id()->str();
    const std::string dst_area = msg->dst_area_id()->str();

    // Sanity: sender must match the claimed src_area
    if (src_area != sender_area_id)
        return;

    // 2. Handle DISPATCH_FORM vs FREE_TEXT
    if (msg->kind() == proto::BilateralKind_FREE_TEXT)
    {
        const auto* ft_payload = msg->body_as_FreeTextPayload();
        if (!ft_payload || !ft_payload->body())
            return;

        const std::uint64_t ts = now_us();

        TelegramRow row;
        row.form_type = "FREE_TEXT";
        row.exchange_id = "";  // free text has no exchange state
        row.train_number = "";
        row.from_sid = src_area;
        row.to_sid = dst_area;
        row.direction = "SENT";
        row.status = "ACCEPTED";
        row.body = ft_payload->body()->str();
        row.timestamp_us = ts;

        db_writer_.write_dispatch_telegram(session_id_, std::move(row));

        // Broadcast back to pair (echo + relay)
        flatbuffers::FlatBufferBuilder fbb(512);
        auto src_off = fbb.CreateString(src_area);
        auto dst_off = fbb.CreateString(dst_area);
        auto sender_off = fbb.CreateString(sender_client_id);
        auto exch_off = fbb.CreateString("");
        auto body_str = fbb.CreateString(ft_payload->body()->str());
        auto ft_off = proto::CreateFreeTextPayload(fbb, body_str);
        auto root = proto::CreateBilateralMessage(
            fbb, src_off, dst_off, proto::TelegramDirection_SENT, proto::BilateralKind_FREE_TEXT,
            proto::BilateralBody_FreeTextPayload, ft_off.Union(), exch_off,
            proto::ExchangeStatus_ACCEPTED, ts, sender_off);
        fbb.Finish(root);

        std::vector<std::uint8_t> out(fbb.GetBufferPointer(),
                                      fbb.GetBufferPointer() + fbb.GetSize());
        auto wire = encode_frame(msg_type::kBilateral, 0, 0, out);
        gateway_.broadcast_to_pair(src_area, dst_area, std::move(wire));
        return;
    }

    // DISPATCH_FORM path
    const auto* df_payload = msg->body_as_DispatchFormPayload();
    if (!df_payload || !df_payload->train_number())
        return;

    const std::string train_number = df_payload->train_number()->str();
    const engine::core::DispatchFormType engine_form = to_engine_form(df_payload->form_type());
    const engine::core::TelegramDirection direction =
        (msg->direction() == proto::TelegramDirection_SENT)
            ? engine::core::TelegramDirection::SENT
            : engine::core::TelegramDirection::RECEIVED;

    // 3. Drive the state machine
    const auto outcome =
        exchanges_.submit_telegram(src_area, dst_area, engine_form, direction, train_number);

    if (outcome.result != TelegramResult::ACCEPTED)
        return;  // rejected — drop; client should not retry without correcting state

    const std::uint64_t ts = now_us();

    // 4. Persist
    {
        TelegramRow row;
        row.form_type = form_type_str(df_payload->form_type());
        row.exchange_id = outcome.exchange_id;
        row.train_number = train_number;
        row.from_sid = src_area;
        row.to_sid = dst_area;
        row.direction = (direction == engine::core::TelegramDirection::SENT) ? "SENT" : "RECEIVED";
        row.status = "ACCEPTED";

        if (df_payload->track_number())
            row.track_number = df_payload->track_number()->str();

        if (const auto* km = df_payload->km_markers())
        {
            row.km_markers.reserve(km->size());
            for (const auto* s : *km)
                if (s)
                    row.km_markers.push_back(s->str());
        }

        row.body = "";  // snapshot JSON — optional, left empty for now
        row.timestamp_us = ts;

        db_writer_.write_dispatch_telegram(session_id_, row);
    }

    // S24/S56 — update EDR track_clear_time
    if (engine_form == engine::core::DispatchFormType::S24 ||
        engine_form == engine::core::DispatchFormType::S56)
    {
        db_writer_.update_edr_track_clear_time(session_id_, train_number, dst_area, ts);
    }

    // 5. Build outbound frame with server-filled fields
    {
        flatbuffers::FlatBufferBuilder fbb(512);
        auto src_off = fbb.CreateString(src_area);
        auto dst_off = fbb.CreateString(dst_area);
        auto sender_off = fbb.CreateString(sender_client_id);
        auto exch_off = fbb.CreateString(outcome.exchange_id);
        auto train_off = fbb.CreateString(train_number);

        std::vector<flatbuffers::Offset<flatbuffers::String>> km_offs;
        if (const auto* km = df_payload->km_markers())
            for (const auto* s : *km)
                if (s)
                    km_offs.push_back(fbb.CreateString(s->str()));
        auto km_vec = fbb.CreateVector(km_offs);

        flatbuffers::Offset<flatbuffers::String> track_off = 0;
        if (df_payload->track_number())
            track_off = fbb.CreateString(df_payload->track_number()->str());

        auto dfp_off = proto::CreateDispatchFormPayload(fbb, df_payload->form_type(), train_off,
                                                        km_vec, track_off);

        auto root = proto::CreateBilateralMessage(
            fbb, src_off, dst_off, msg->direction(), proto::BilateralKind_DISPATCH_FORM,
            proto::BilateralBody_DispatchFormPayload, dfp_off.Union(), exch_off,
            to_proto_status(outcome.new_status), ts, sender_off);
        fbb.Finish(root);

        std::vector<std::uint8_t> out(fbb.GetBufferPointer(),
                                      fbb.GetBufferPointer() + fbb.GetSize());
        auto wire = encode_frame(msg_type::kBilateral, 0, 0, out);
        gateway_.broadcast_to_pair(src_area, dst_area, std::move(wire));
    }
}

}  // namespace server
