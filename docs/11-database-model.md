# Database Model

## Decision

**One PostgreSQL instance, three schemas.**

- Schema `fleet` — static reference data, loaded from config files at server startup. Read-mostly.
- Schema `session` — live operational data written during simulation. Write-heavy (event log).
- Schema `pip` — live PIP (Train Identification Panel) state; one row per track section. Written exclusively by the PIP_WRITER thread.

Resolves open questions OQ-7 and OQ-8 from `02-system-requirements.md`.

**Rationale for one instance:** At MVP scale (9 stations, up to ~36 clients) there is no operational justification for multiple PostgreSQL processes. One instance means one backup job, one connection pool, one monitoring target. Splitting into multiple instances is a deployment change only — no schema changes required — so the decision is reversible at any time.

**Why a separate `pip` schema and not a table in `session`:** The `pip` schema is written by a dedicated PIP_WRITER thread that must not share a write path with the append-only `session.events` log. Keeping it in its own schema enforces ownership — only PIP_WRITER touches `pip.*` — and makes a future migration to a separate PostgreSQL instance a connection-string change, not a refactor.

**EDR integration:** A new native C++ component will be written (not adapted from the C# prototype). It owns the `fleet.timetable_templates` table and the `session.edr_entries` table. It communicates with the engine via direct in-process call or Unix socket (Channel 2, same as defined in `09-communication-contract.md`).

---

## Schema `fleet`

Static reference data. Populated by the server at startup by importing from JSON config files. Never written during an active session.

```sql
-- Schema: fleet

CREATE TABLE fleet.vehicles (
    gid             TEXT        PRIMARY KEY,           -- e.g. VEH-GGO-ET22-001-0000001
    pid             TEXT        NOT NULL,
    type            TEXT        NOT NULL,              -- LOCOMOTIVE | EMU_UNIT | PASSENGER_WAGON | ...
    subtype         TEXT,
    display_name    TEXT        NOT NULL,
    length_m        REAL        NOT NULL,
    axle_count      INTEGER     NOT NULL,
    mass_empty_t    REAL        NOT NULL,
    mass_gross_t    REAL,                              -- loaded mass for freight; NULL means use mass_empty_t
    max_speed_kmh   INTEGER     NOT NULL,
    power_kw        REAL,                              -- NULL for unpowered vehicles
    traction_force_kn REAL,                            -- NULL for unpowered vehicles
    traction_capable BOOLEAN    NOT NULL DEFAULT FALSE,
    traction_status  TEXT,                             -- OPERATIONAL | DEFECTIVE; NULL for non-traction vehicles
    multiple_coupling_capable BOOLEAN,                 -- traction-capable type capability; NULL = unknown or not applicable
    braking_lambda_pct INTEGER  NOT NULL DEFAULT 100   -- UIC lambda %; see docs/10-vehicle-model.md
);

CREATE TABLE fleet.train_definitions (
    gid             TEXT        PRIMARY KEY,           -- e.g. TRN-GGO-IC12345-0000100
    pid             TEXT        NOT NULL,              -- e.g. IC 12345
    display_name    TEXT        NOT NULL,
    train_category  TEXT        NOT NULL DEFAULT 'PASSENGER',
                                -- PASSENGER | FREIGHT | MAINTENANCE
                                -- Determines default icon and handling rules.
                                -- Matches TrainCategory enum in engine/core/types.hpp.
    classification  TEXT,                              -- PKP internal classification code;
                                                       -- free-form, e.g. "TLK", "IC", "pospieszny"
    supplement      TEXT,                              -- Additional operational annotation shown in
                                                       -- PIP "Uzupełnienie" column, e.g.
                                                       -- "z towarem niebezpiecznym", "TD"
    description     TEXT                               -- Free-form notes visible to dispatchers
);

CREATE TABLE fleet.train_definition_vehicles (
    definition_gid  TEXT        NOT NULL REFERENCES fleet.train_definitions(gid),
    position        INTEGER     NOT NULL,              -- 0 = front
    vehicle_gid     TEXT        NOT NULL REFERENCES fleet.vehicles(gid),
    PRIMARY KEY (definition_gid, position)
);

-- Timetable template: one row per train × station passage.
-- Imported from timetable JSON config file at session start.
CREATE TABLE fleet.timetable_templates (
    id              SERIAL      PRIMARY KEY,
    train_number    TEXT        NOT NULL,              -- e.g. IC 12345
    train_definition_gid TEXT   REFERENCES fleet.train_definitions(gid),
    station_sid     TEXT        NOT NULL,              -- e.g. GGO
    operating_point_id   TEXT,                              -- which sub-post sees this train (NULL = whole station)
    scheduled_arrival    INTERVAL,                     -- offset from session start (NULL for first origin)
    scheduled_departure  INTERVAL  NOT NULL,
    track_number    TEXT,                              -- planned platform/track
    stop_type       TEXT        NOT NULL DEFAULT 'COMMERCIAL'  -- COMMERCIAL | TECHNICAL | PASS_THROUGH
);

CREATE INDEX ON fleet.timetable_templates (station_sid, scheduled_departure);
```

---

## Schema `session`

Operational data written during simulation. One session row per simulation run.

```sql
-- Schema: session

CREATE TABLE session.sessions (
    id              UUID        PRIMARY KEY DEFAULT gen_random_uuid(),
    display_name    TEXT        NOT NULL,
    status          TEXT        NOT NULL DEFAULT 'CREATED',
                                -- CREATED | STARTED | PAUSED | ENDED
    schema_version  INTEGER     NOT NULL,
    started_at      TIMESTAMPTZ,
    ended_at        TIMESTAMPTZ,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT now()
);

-- Append-only event log. Never updated, never deleted during a session.
CREATE TABLE session.events (
    id              BIGSERIAL   PRIMARY KEY,
    session_id      UUID        NOT NULL REFERENCES session.sessions(id),
    event_type      SMALLINT    NOT NULL,              -- mirrors DOMAIN_EVENT event_type byte
    event_id        BIGINT      NOT NULL,              -- server monotonic counter
    timestamp_us    BIGINT      NOT NULL,              -- microseconds since session epoch
    object_gid      TEXT,                              -- NULL for session-level events
    payload         BYTEA       NOT NULL               -- FlatBuffers-serialized body
);

CREATE INDEX ON session.events (session_id, event_id);
CREATE INDEX ON session.events (session_id, object_gid) WHERE object_gid IS NOT NULL;

-- Periodic engine state snapshots for fast reconnect and replay.
CREATE TABLE session.snapshots (
    id              BIGSERIAL   PRIMARY KEY,
    session_id      UUID        NOT NULL REFERENCES session.sessions(id),
    seq_cursor      BIGINT      NOT NULL,              -- event_id of last included event
    timestamp_us    BIGINT      NOT NULL,
    payload         BYTEA       NOT NULL               -- FlatBuffers Snapshot table
);

CREATE INDEX ON session.snapshots (session_id, seq_cursor DESC);

-- Live EDR: one row per train × station passage within a session.
-- Generated from fleet.timetable_templates at session start.
-- Editable by the EDR operator during the session.
CREATE TABLE session.edr_entries (
    id              BIGSERIAL   PRIMARY KEY,
    session_id      UUID        NOT NULL REFERENCES session.sessions(id),
    train_number    TEXT        NOT NULL,
    station_sid     TEXT        NOT NULL,
    operating_point_id   TEXT,
    scheduled_arrival    INTERVAL,
    actual_arrival       INTERVAL,
    scheduled_departure  INTERVAL  NOT NULL,
    actual_departure     INTERVAL,
    track_number    TEXT,
    stop_type       TEXT        NOT NULL DEFAULT 'COMMERCIAL',
                                -- COMMERCIAL | TECHNICAL | PASS_THROUGH
    status          TEXT        NOT NULL DEFAULT 'PENDING',
                                -- PENDING | ARRIVED | DEPARTED | SKIPPED | CANCELLED
    track_clear_time INTERVAL,                         -- "Droga wolna" — time when the dispatcher
                                                       -- confirmed the line/track is clear after
                                                       -- departure. Set by S-form dispatch process;
                                                       -- NULL until confirmed. Duplicate-confirmation
                                                       -- is rejected by the server with a warning.
    notes           TEXT,
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX ON session.edr_entries (session_id, station_sid, scheduled_departure);
CREATE INDEX ON session.edr_entries (session_id, train_number);

-- Dispatch telegram log (S-form exchange between neighbouring LCS).
-- One row per telegram in a dispatch exchange.
-- The full S-form state machine is documented in docs/15-dispatch-forms.md.
CREATE TABLE session.dispatch_telegrams (
    id              BIGSERIAL   PRIMARY KEY,
    session_id      UUID        NOT NULL REFERENCES session.sessions(id),
    form_type       TEXT        NOT NULL,              -- S2 | S24 | S25 | S26 | S35 | S51 |
                                                       -- S52 | S55 | S56 | S76
    exchange_id     UUID        NOT NULL,              -- groups all telegrams of one dispatch
                                                       -- exchange (question + reply + confirm)
    train_number    TEXT        NOT NULL,
    from_sid        TEXT        NOT NULL,              -- sending LCS
    to_sid          TEXT        NOT NULL,              -- receiving LCS
    direction       TEXT        NOT NULL,              -- SENT | RECEIVED
    status          TEXT        NOT NULL DEFAULT 'PENDING',
                                -- PENDING | CONFIRMED | REJECTED | SUPERSEDED
    track_number    TEXT,                              -- tor szlakowy / stacyjny
    km_markers      TEXT[],                            -- level crossing km positions notified,
                                                       -- e.g. {"210.394","212.705"}
    body            TEXT        NOT NULL,              -- rendered telegram text (for log/display)
    timestamp_us    BIGINT      NOT NULL               -- session epoch offset
);

CREATE INDEX ON session.dispatch_telegrams (session_id, exchange_id);
CREATE INDEX ON session.dispatch_telegrams (session_id, train_number);
CREATE INDEX ON session.dispatch_telegrams (session_id, from_sid, to_sid);

-- Who controls which operating point at any point in time.
CREATE TABLE session.operating_point_assignments (
    id              BIGSERIAL   PRIMARY KEY,
    session_id      UUID        NOT NULL REFERENCES session.sessions(id),
    operating_point_id   TEXT        NOT NULL,
    station_sid     TEXT        NOT NULL,
    client_id       TEXT        NOT NULL,
    assigned_at     TIMESTAMPTZ NOT NULL DEFAULT now(),
    released_at     TIMESTAMPTZ                        -- NULL = currently held
);

CREATE INDEX ON session.operating_point_assignments (session_id, operating_point_id, released_at)
    WHERE released_at IS NULL;

-- Chat log (resolves Q-COM-6: separate table, not mixed with domain events).
-- Kept separate to allow independent retention and query without scanning event BYTEA payloads.
CREATE TABLE session.chat_log (
    id              BIGSERIAL   PRIMARY KEY,
    session_id      UUID        NOT NULL REFERENCES session.sessions(id),
    sender_id       TEXT        NOT NULL,
    target_type     TEXT        NOT NULL,              -- BROADCAST | STATION | PLAYER
    target_id       TEXT,                              -- NULL when BROADCAST
    body            TEXT        NOT NULL,
    timestamp_us    BIGINT      NOT NULL
);

CREATE INDEX ON session.chat_log (session_id, timestamp_us);
```

---

## Schema `pip`

Live PIP state. Written exclusively by the PIP_WRITER thread. Never written by the ENGINE, DISPATCHER, or DB_WRITER. Provides fast point-in-time reads for reconnecting clients without replaying the full event log.

```sql
-- Schema: pip

-- One row per track section per session.
-- UPSERT on every TrackOccupancyChanged / train number assignment.
CREATE TABLE pip.track_state (
    session_id      UUID        NOT NULL REFERENCES session.sessions(id),
    section_gid     TEXT        NOT NULL,
    trains          JSONB       NOT NULL DEFAULT '[]',
                                -- Array of TrainSlot objects:
                                -- [
                                --   {
                                --     "number": "IC1234",      -- up to 6 chars
                                --     "has_extra_info": false, -- show "!" badge
                                --     "manually_placed": false,-- blink on client
                                --     "entry_side": "LEFT"     -- LEFT | RIGHT
                                --   }
                                -- ]
                                -- 0 items = track empty / no number assigned
                                -- 1 item  = standard display
                                -- 2 items = alternating display (client animates 1 s)
                                -- 3+ items = column display (client or server aggregates)
    path_confirmed  BOOLEAN     NOT NULL DEFAULT FALSE,
                                -- TRUE when a confirmed route is set over this section;
                                -- client renders track in green and number in track colour
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now(),
    PRIMARY KEY (session_id, section_gid)
);

CREATE INDEX ON pip.track_state (session_id);
```

### Train number lifecycle

| Event | Source | PIP_WRITER action |
|---|---|---|
| Pociąg wjeżdża na odcinek | ENGINE `TrackOccupancyChanged` | UPSERT: append `TrainSlot` to `trains` |
| Pociąg wyjeżdża z odcinka | ENGINE `TrackOccupancyChanged` | UPSERT: remove matching `TrainSlot` from `trains` |
| Pociąg przekracza granicę LCS | ENGINE `TrainCrossedLcsBoundary` | UPSERT target section in `pip.track_state` only — EDR is independent, PIP_WRITER does not touch `session.edr_entries` |
| Dyżurny wpisuje numer ręcznie | COMMAND `AssignTrainNumber` | UPSERT: add `TrainSlot{manually_placed=true}` |
| Dyżurny usuwa numer ręcznie | COMMAND `RemoveTrainNumber` | UPSERT: remove `TrainSlot` |
| Powiązana informacja dodatkowa | COMMAND `SetTrainExtraInfo` | UPSERT: set `has_extra_info=true` on matching slot |

### Column display aggregation

When `trains` contains more than 3 entries (line-block multi-section tracking), PIP_WRITER stores the raw full list. The client is responsible for rendering the column display: it shows `trains[0]`, a synthetic middle entry `"{N}*"` where N = `trains.size() - 2`, and `trains.back()`. The server never truncates the list — storing the full list allows the client to reconstruct any view and simplifies server logic.

---

## EDR row volume estimate (Trójmiasto scenario)

| Station                  | Through-trains/day | EDR rows/session |
|--------------------------|--------------------|-----------------|
| Gdynia Chylonia          | ~60                | ~60             |
| Gdynia Postojowa/Grabówek| ~80                | ~80             |
| Gdynia Główna            | ~180               | ~180            |
| Gdynia Orłowo            | ~60                | ~60             |
| Sopot                    | ~140               | ~140            |
| Gdańsk Oliwa             | ~100               | ~100            |
| Gdańsk Wrzeszcz          | ~140               | ~140            |
| Gdańsk Główny            | ~200               | ~200            |
| Gdańsk Orunia            | ~60                | ~60             |
| **Total**                |                    | **~1 020 rows** |

A full Trójmiasto session generates ~1 000 EDR rows. This fits entirely in PostgreSQL's shared buffer cache — no partitioning or archiving needed for MVP.

---

## Retention policy (MVP)

| Table               | Policy                                               |
|---------------------|------------------------------------------------------|
| `session.events`    | Keep all rows for active + last 10 completed sessions. Archive or delete older. |
| `session.snapshots` | Keep only the 3 most recent snapshots per session (older ones are superseded). |
| `session.edr_entries` | Retain for the lifetime of the session + 30 days.  |
| `session.dispatch_telegrams` | Retain for the lifetime of the session + 30 days. Required for audit trail of S-form exchanges. |
| `session.chat_log`  | Retain for the lifetime of the session + 30 days.   |
| `pip.track_state`   | Delete all rows for a session when the session ends. No archiving needed — state is reconstructable from `session.events`. |
| `fleet.*`           | Never deleted; refreshed only on explicit re-import. |

Retention is enforced by a maintenance job (cron or pg_cron), not by application code.

---

## Open questions

- Q-DB-1: Should `edr_entries` store times as `INTERVAL` (offset from session epoch) or as `TIMESTAMPTZ`? INTERVAL is natural for a simulator where wall-clock time is simulated; TIMESTAMPTZ is easier to display but requires storing the session epoch.
- Q-DB-2: Is a dedicated `session.alarms` table needed, or is the alarm lifecycle fully covered by `AlarmRaised` / `AlarmCleared` events in `session.events`?
