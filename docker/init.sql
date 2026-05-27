-- docker/init.sql
-- Initialised automatically when the PostgreSQL container first starts.
-- The database user and database name are set via environment variables
-- in docker-compose.yml (POSTGRES_USER, POSTGRES_DB).

-- ── Extensions ───────────────────────────────────────────────────────────────
CREATE EXTENSION IF NOT EXISTS "pgcrypto";   -- for gen_random_uuid()

-- ── Schemas ──────────────────────────────────────────────────────────────────
CREATE SCHEMA IF NOT EXISTS fleet;
CREATE SCHEMA IF NOT EXISTS session;

-- ── Schema: fleet ────────────────────────────────────────────────────────────
-- Static reference data.  Populated at server startup from JSON config files.
-- Never modified during an active simulation session.

CREATE TABLE IF NOT EXISTS fleet.vehicles (
    gid               TEXT    PRIMARY KEY,           -- e.g. VEH-GGO-ET22-001-0000001
    pid               TEXT    NOT NULL,
    type              TEXT    NOT NULL,              -- LOCOMOTIVE | EMU_UNIT | PASSENGER_WAGON | ...
    subtype           TEXT,
    display_name      TEXT    NOT NULL,
    length_m          REAL    NOT NULL,
    axle_count        INTEGER NOT NULL,
    mass_empty_t      REAL    NOT NULL,
    mass_gross_t      REAL,                          -- loaded mass (freight); NULL = use mass_empty_t
    max_speed_kmh     INTEGER NOT NULL,
    power_kw          REAL,                          -- NULL for unpowered vehicles
    traction_force_kn REAL,                          -- NULL for unpowered vehicles
    traction_capable  BOOLEAN NOT NULL DEFAULT FALSE,
    traction_status   TEXT,                          -- OPERATIONAL | DEFECTIVE; NULL for non-traction vehicles
    multiple_coupling_capable BOOLEAN,               -- traction-capable type capability; NULL = unknown/not applicable
    braking_lambda_pct INTEGER NOT NULL DEFAULT 100  -- UIC lambda %; see docs/10-vehicle-model.md
);

CREATE TABLE IF NOT EXISTS fleet.train_definitions (
    gid          TEXT PRIMARY KEY,                   -- e.g. TRN-GGO-IC12345-0000100
    pid          TEXT NOT NULL,                      -- e.g. IC 12345
    display_name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS fleet.train_definition_vehicles (
    definition_gid TEXT    NOT NULL REFERENCES fleet.train_definitions(gid),
    position       INTEGER NOT NULL,                 -- 0 = front of train
    vehicle_gid    TEXT    NOT NULL REFERENCES fleet.vehicles(gid),
    PRIMARY KEY (definition_gid, position)
);

CREATE TABLE IF NOT EXISTS fleet.timetable_templates (
    id                   SERIAL  PRIMARY KEY,
    train_number         TEXT    NOT NULL,           -- e.g. IC 12345
    train_definition_gid TEXT    REFERENCES fleet.train_definitions(gid),
    station_sid          TEXT    NOT NULL,           -- e.g. GGO
    operating_point_id   TEXT,                       -- NULL = whole station sees this train
    scheduled_arrival    INTERVAL,                   -- offset from session start; NULL for first origin
    scheduled_departure  INTERVAL NOT NULL,
    track_number         TEXT,
    stop_type            TEXT    NOT NULL DEFAULT 'COMMERCIAL'  -- COMMERCIAL | TECHNICAL | PASS_THROUGH
);

CREATE INDEX IF NOT EXISTS idx_timetable_station_dep
    ON fleet.timetable_templates (station_sid, scheduled_departure);

-- ── Schema: session ───────────────────────────────────────────────────────────
-- Live operational data.  Written during simulation.  One row in sessions per run.

CREATE TABLE IF NOT EXISTS session.sessions (
    id             UUID        PRIMARY KEY DEFAULT gen_random_uuid(),
    display_name   TEXT        NOT NULL,
    status         TEXT        NOT NULL DEFAULT 'CREATED',
                               -- CREATED | STARTED | PAUSED | ENDED
    schema_version INTEGER     NOT NULL,
    started_at     TIMESTAMPTZ,
    ended_at       TIMESTAMPTZ,
    created_at     TIMESTAMPTZ NOT NULL DEFAULT now()
);

-- Append-only event log.  Never updated, never deleted during an active session.
CREATE TABLE IF NOT EXISTS session.events (
    id           BIGSERIAL PRIMARY KEY,
    session_id   UUID      NOT NULL REFERENCES session.sessions(id) ON DELETE CASCADE,
    event_type   SMALLINT  NOT NULL,   -- mirrors DOMAIN_EVENT event_type byte (see docs/09)
    event_id     BIGINT    NOT NULL,   -- server monotonic counter
    timestamp_us BIGINT    NOT NULL,   -- microseconds since session epoch
    object_gid   TEXT,                 -- NULL for session-level events
    payload      BYTEA     NOT NULL    -- FlatBuffers-serialized body
);

CREATE INDEX IF NOT EXISTS idx_events_session_event
    ON session.events (session_id, event_id);
CREATE INDEX IF NOT EXISTS idx_events_session_object
    ON session.events (session_id, object_gid) WHERE object_gid IS NOT NULL;

-- Periodic engine snapshots for fast reconnect and replay.
CREATE TABLE IF NOT EXISTS session.snapshots (
    id           BIGSERIAL PRIMARY KEY,
    session_id   UUID      NOT NULL REFERENCES session.sessions(id) ON DELETE CASCADE,
    seq_cursor   BIGINT    NOT NULL,   -- event_id of last event included
    timestamp_us BIGINT    NOT NULL,
    payload      BYTEA     NOT NULL    -- FlatBuffers Snapshot table (see proto/snapshot.fbs)
);

CREATE INDEX IF NOT EXISTS idx_snapshots_session_cursor
    ON session.snapshots (session_id, seq_cursor DESC);

-- Live EDR: one row per train × station passage within a session.
CREATE TABLE IF NOT EXISTS session.edr_entries (
    id                   BIGSERIAL   PRIMARY KEY,
    session_id           UUID        NOT NULL REFERENCES session.sessions(id) ON DELETE CASCADE,
    train_number         TEXT        NOT NULL,
    station_sid          TEXT        NOT NULL,
    operating_point_id   TEXT,
    scheduled_arrival    INTERVAL,
    actual_arrival       INTERVAL,
    scheduled_departure  INTERVAL    NOT NULL,
    actual_departure     INTERVAL,
    track_number         TEXT,
    track_clear_time     INTERVAL,              -- "Droga wolna" — set by S24/S56 in DispatchCoordinator
    stop_type            TEXT        NOT NULL DEFAULT 'COMMERCIAL',
    status               TEXT        NOT NULL DEFAULT 'PENDING',
                                     -- PENDING | ARRIVED | DEPARTED | SKIPPED | CANCELLED
    notes                TEXT,
    updated_at           TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS idx_edr_station
    ON session.edr_entries (session_id, station_sid, scheduled_departure);
CREATE INDEX IF NOT EXISTS idx_edr_train
    ON session.edr_entries (session_id, train_number);

-- Operating point ownership — who controls which operating point.
CREATE TABLE IF NOT EXISTS session.operating_point_assignments (
    id                 BIGSERIAL   PRIMARY KEY,
    session_id         UUID        NOT NULL REFERENCES session.sessions(id) ON DELETE CASCADE,
    operating_point_id TEXT        NOT NULL,
    station_sid        TEXT        NOT NULL,
    client_id     TEXT        NOT NULL,
    assigned_at   TIMESTAMPTZ NOT NULL DEFAULT now(),
    released_at   TIMESTAMPTZ          -- NULL = currently held
);

CREATE INDEX IF NOT EXISTS idx_operating_point_active
    ON session.operating_point_assignments (session_id, operating_point_id, released_at)
    WHERE released_at IS NULL;

-- Chat log — kept separate from domain events for independent retention and querying.
CREATE TABLE IF NOT EXISTS session.chat_log (
    id           BIGSERIAL PRIMARY KEY,
    session_id   UUID      NOT NULL REFERENCES session.sessions(id) ON DELETE CASCADE,
    sender_id    TEXT      NOT NULL,
    target_type  TEXT      NOT NULL,   -- BROADCAST | STATION | PLAYER
    target_id    TEXT,                 -- NULL when BROADCAST
    body         TEXT      NOT NULL,
    timestamp_us BIGINT    NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_chat_session
    ON session.chat_log (session_id, timestamp_us);

-- Dispatch channel — dispatch telegrams (S-forms and free-text between neighbouring areas).
CREATE TABLE IF NOT EXISTS session.dispatch_telegrams (
    id              BIGSERIAL   PRIMARY KEY,
    session_id      UUID        NOT NULL REFERENCES session.sessions(id) ON DELETE CASCADE,
    form_type       TEXT        NOT NULL,   -- S2 | S24 | S25 | S26 | S55 | S56 | FREE_TEXT …
    exchange_id     TEXT        NOT NULL,   -- e.g. "exch-0000001"
    train_number    TEXT        NOT NULL,
    from_sid        TEXT        NOT NULL,   -- source dispatch area id
    to_sid          TEXT        NOT NULL,   -- destination dispatch area id
    direction       TEXT        NOT NULL,   -- SENT | RECEIVED (from src perspective)
    status          TEXT        NOT NULL DEFAULT 'PENDING',
                                            -- PENDING | ACCEPTED | REJECTED | CLOSED
    track_number    TEXT,
    km_markers      TEXT[],
    body            TEXT        NOT NULL,   -- JSON snapshot of the telegram payload
    timestamp_us    BIGINT      NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_dispatch_telegrams_session_exchange
    ON session.dispatch_telegrams (session_id, exchange_id);
CREATE INDEX IF NOT EXISTS idx_dispatch_telegrams_session_train
    ON session.dispatch_telegrams (session_id, train_number);
CREATE INDEX IF NOT EXISTS idx_dispatch_telegrams_session_areas
    ON session.dispatch_telegrams (session_id, from_sid, to_sid);

-- ── Schema: pip ───────────────────────────────────────────────────────────────
-- PIP (Pulse Interval Processing) track-state cache.
-- Written by PipWriter on every engine tick; one row per session × section.

CREATE SCHEMA IF NOT EXISTS pip;

CREATE TABLE IF NOT EXISTS pip.track_state (
    session_id  UUID        NOT NULL REFERENCES session.sessions(id) ON DELETE CASCADE,
    section_gid TEXT        NOT NULL,
    trains      JSONB       NOT NULL DEFAULT '[]'::jsonb,
    updated_at  TIMESTAMPTZ NOT NULL DEFAULT now(),
    PRIMARY KEY (session_id, section_gid)
);
