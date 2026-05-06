# Database Model

## Decision

**One PostgreSQL instance, two schemas.**

- Schema `fleet` — static reference data, loaded from config files at server startup. Read-mostly.
- Schema `session` — live operational data written during simulation. Write-heavy (event log).

Resolves open questions OQ-7 and OQ-8 from `02-system-requirements.md`.

**Rationale for one instance:** At MVP scale (9 stations, up to ~36 clients) there is no operational justification for two PostgreSQL processes. One instance means one backup job, one connection pool, one monitoring target. Splitting into two instances is a deployment change only — no schema changes required — so the decision is reversible at any time.

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
    max_speed_kmh   INTEGER     NOT NULL,
    power_kw        REAL,                              -- NULL for unpowered vehicles
    traction_force_kn REAL                             -- NULL for unpowered vehicles
);

CREATE TABLE fleet.train_definitions (
    gid             TEXT        PRIMARY KEY,           -- e.g. TRN-GGO-IC12345-0000100
    pid             TEXT        NOT NULL,              -- e.g. IC 12345
    display_name    TEXT        NOT NULL
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
    posterunek_id   TEXT,                              -- which sub-post sees this train (NULL = whole station)
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
    posterunek_id   TEXT,
    scheduled_arrival    INTERVAL,
    actual_arrival       INTERVAL,
    scheduled_departure  INTERVAL  NOT NULL,
    actual_departure     INTERVAL,
    track_number    TEXT,
    stop_type       TEXT        NOT NULL DEFAULT 'COMMERCIAL',
    status          TEXT        NOT NULL DEFAULT 'PENDING',
                                -- PENDING | ARRIVED | DEPARTED | SKIPPED | CANCELLED
    notes           TEXT,
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT now()
);

CREATE INDEX ON session.edr_entries (session_id, station_sid, scheduled_departure);
CREATE INDEX ON session.edr_entries (session_id, train_number);

-- Who controls which posterunek at any point in time.
CREATE TABLE session.posterunek_assignments (
    id              BIGSERIAL   PRIMARY KEY,
    session_id      UUID        NOT NULL REFERENCES session.sessions(id),
    posterunek_id   TEXT        NOT NULL,
    station_sid     TEXT        NOT NULL,
    client_id       TEXT        NOT NULL,
    assigned_at     TIMESTAMPTZ NOT NULL DEFAULT now(),
    released_at     TIMESTAMPTZ                        -- NULL = currently held
);

CREATE INDEX ON session.posterunek_assignments (session_id, posterunek_id, released_at)
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
| `session.chat_log`  | Retain for the lifetime of the session + 30 days.   |
| `fleet.*`           | Never deleted; refreshed only on explicit re-import. |

Retention is enforced by a maintenance job (cron or pg_cron), not by application code.

---

## Open questions

- Q-DB-1: Should `edr_entries` store times as `INTERVAL` (offset from session epoch) or as `TIMESTAMPTZ`? INTERVAL is natural for a simulator where wall-clock time is simulated; TIMESTAMPTZ is easier to display but requires storing the session epoch.
- Q-DB-2: Is a dedicated `session.alarms` table needed, or is the alarm lifecycle fully covered by `AlarmRaised` / `AlarmCleared` events in `session.events`?
