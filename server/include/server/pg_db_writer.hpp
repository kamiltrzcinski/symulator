// server/include/server/pg_db_writer.hpp

#pragma once

#include "server/db_writer.hpp"

#include <pqxx/pqxx>

#include <mutex>
#include <string>

namespace server
{

class PgDbWriter : public IDbWriter
{
public:
    explicit PgDbWriter(const std::string& connection_string);

    std::string init_session(const std::string& display_name, int schema_version) override;

    int64_t seed_edr_entries_for_operating_day(const std::string& session_id,
                                               int iso_weekday) override;

    void write_domain_event(const std::string& session_id, DomainEventRow row) override;

    void write_dispatch_telegram(const std::string& session_id, TelegramRow row) override;

    void update_edr_track_clear_time(const std::string& session_id, const std::string& train_number,
                                     std::uint64_t station_uid,
                                     std::uint64_t timestamp_us) override;

    void update_edr_departure(const std::string& session_id, const std::string& train_number,
                              std::uint64_t station_uid, std::uint64_t timestamp_us) override;

    void update_edr_arrival(const std::string& session_id, const std::string& train_number,
                            std::uint64_t station_uid, std::uint64_t timestamp_us) override;

    int64_t append_edr_journal_entry(const std::string& session_id,
                                     EdrJournalEntryRow row) override;

    void update_edr_journal_entry_status(const std::string& session_id, int64_t entry_id,
                                         const std::string& status,
                                         const std::optional<std::string>& notes) override;

    void upsert_pip_track_state(const std::string& session_id, std::uint64_t section_uid,
                                const std::string& trains_json) override;

    void save_snapshot(const std::string& session_id, int64_t seq_cursor, int64_t timestamp_us,
                       const std::vector<std::uint8_t>& payload) override;

    void append_chat_message(const std::string& session_id, const std::string& sender_id,
                             const std::string& target_type,
                             const std::optional<std::string>& target_id, const std::string& body,
                             int64_t timestamp_us) override;

    int64_t assign_operating_point(const std::string& session_id,
                                   const std::string& operating_point_id,
                                   const std::string& station_sid,
                                   const std::string& client_id) override;

    void release_operating_point(const std::string& session_id,
                                 const std::string& operating_point_id,
                                 const std::string& client_id) override;

    void upsert_timetable_template(const std::string& train_number, std::uint64_t station_uid,
                                   const std::optional<std::string>& operating_point_id,
                                   const std::optional<std::string>& scheduled_arrival_secs,
                                   const std::string& scheduled_departure_secs,
                                   const std::optional<std::string>& track_number,
                                   const std::string& stop_type,
                                   const std::vector<int>& operating_days) override;

private:
    pqxx::connection conn_;
    std::string session_uuid_;
    std::mutex mu_;
};

}  // namespace server
