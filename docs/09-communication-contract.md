# Communication Contract v1

## Scope

This document defines the binary framing, message type catalog, and payload structure for Channel 1 (Operator Client ↔ Session Server). It resolves P0 backlog item "Define command/event/snapshot communication contract".

Channel 2 (EDR ↔ Engine, intra-server) reuses the same framing if a socket boundary is needed; otherwise direct function calls share the same payload types.

---

## Binary frame layout

Every message on the wire, in both directions, is a single frame:

```
Offset  Size  Field          Notes
──────  ────  ─────────────  ─────────────────────────────────────────────
 0       2    magic          0x5352  ('S','R') — rejects stray connections
 2       1    msg_type       frame family (see message type table below)
 3       1    flags          bit 0 = IS_LAST_CHUNK; bits 1-7 reserved (0)
 4       4    seq_id         uint32, monotonically increasing per sender;
                             wraps at 2^32; used to correlate ACK/NAK
 8       4    payload_len    uint32, bytes; 0 for header-only messages;
                             max 65 535 bytes per frame (MVP cap)
12       4    crc32          CRC-32/ISO-HDLC over bytes [0..11] + payload
16       N    payload        serialized body (see per-type sections below)
```

Total header overhead: **16 bytes**.

If `payload_len` exceeds 65 535 the sender MUST split the logical message into multiple frames with `IS_LAST_CHUNK = 0` on all but the final frame. The receiver reassembles before dispatching. (Used only for `SNAPSHOT_CHUNK`.)

---

## Serialization format

**Chosen: FlatBuffers** (schema files in `proto/` directory)

| Option               | Decision                                                               |
|----------------------|------------------------------------------------------------------------|
| Raw packed C++ structs | Fastest, but any field addition breaks wire compatibility. Ruled out. |
| MessagePack          | Simple, no schema. Rejected: no compile-time type safety, harder to version. |
| Protobuf             | Good but copies data. Viable fallback if FlatBuffers build is blocked. |
| **FlatBuffers**      | **Selected.** Zero-copy reads, schema = contract document, C++17/20 native, forward-compatible (new fields don't break old readers). `.fbs` files in `proto/` are the canonical contract. |

The `.fbs` schema files are the single source of truth. Any change to a payload structure requires updating the schema and bumping `schemaVersion` in the relevant table.

---

## Message type table

```
Value  Name                Direction          Category
─────  ──────────────────  ─────────────────  ────────────────
0x01   HANDSHAKE           C → S              Session lifecycle
0x02   HANDSHAKE_ACK       S → C              Session lifecycle
0x03   HEARTBEAT           C ↔ S              Liveness
0x04   HEARTBEAT_ACK       C ↔ S              Liveness
0x10   COMMAND             C → S              Operator action
0x11   COMMAND_ACK         S → C              Operator action
0x12   COMMAND_NAK         S → C              Operator action
0x20   DOMAIN_EVENT        S → C (broadcast)  State change
0x30   SNAPSHOT_REQUEST    C → S              Reconnect / join
0x31   SNAPSHOT_CHUNK      S → C              Reconnect / join
0x40   SESSION_NOTICE      S → C              Session lifecycle
0x50   TAKEOVER_REQUEST    C → S              Ownership
0x51   TAKEOVER_RESPONSE   S → C              Ownership
0x60   CHAT_MESSAGE        C → S → C(s)       Communication
0x61   BILATERAL_MESSAGE   C ↔ S ↔ C(pair)    Communication
0x70   VOICE_CHAN_JOIN      C → S              Communication
0x71   VOICE_CHAN_LEAVE     C → S              Communication
0x72   VOICE_CHAN_STATE     S → C (broadcast)  Communication
```

`msg_type` is 1 byte (256 slots). Currently allocated: 18. Ranges 0x80–0xFF reserved for future use.

### Type hierarchy and capacity

`msg_type` is a **family discriminator**, not the only discriminator in the protocol.

- `COMMAND (0x10)` carries `cmd_type` (1 byte)
- `DOMAIN_EVENT (0x20)` carries `event_type` (1 byte)
- other families use dedicated payload tables per message

This gives practical capacity far beyond 256 logical operations while keeping the frame header compact:

- frame families: up to 256
- command subtypes: up to 256
- event subtypes: up to 256

In practice this already covers current and near-future scope. If a future family needs more than 256 subtypes, that family can add an internal `uint16 ext_type` in its payload without changing the 16-byte transport header.

---

## Session lifecycle

### Connect and handshake

```
Client                          Server
  │── HANDSHAKE ──────────────▶ │  version, player_id, auth_token
  │◀─ HANDSHAKE_ACK ───────────  │  session_id, assigned_posterunki[], server_tick_hz
  │── SNAPSHOT_REQUEST ────────▶ │  (immediately after handshake)
  │◀─ SNAPSHOT_CHUNK (N) ───────  │  IS_LAST_CHUNK=0 on all but final
  │◀─ SNAPSHOT_CHUNK (last) ────  │  IS_LAST_CHUNK=1; seq_cursor included
  │                               │  ← normal operation starts here
```

`seq_cursor` in the final `SNAPSHOT_CHUNK` tells the client the `seq_id` of the most recent `DOMAIN_EVENT` included in the snapshot. Any event with `seq_id > seq_cursor` received after this point is new.

### Reconnect

Identical to connect. Server rebuilds snapshot from current engine state. No special reconnect handshake message is needed — a reconnecting client simply opens a new TCP connection and repeats the handshake.

### Heartbeat

Sent by each side every **5 s** when no other frame has been sent. The receiver replies with `HEARTBEAT_ACK` echoing the sender's `seq_id`. A 15 s silence on either side triggers a reconnect attempt by the client.

### Graceful disconnect

Client closes the TCP connection. Server marks the client offline and releases station ownership after a configurable grace period (default: 30 s).

---

## Commands (C → S, msg_type = 0x10)

The `COMMAND` payload begins with a 1-byte `cmd_type` field followed by the FlatBuffers body.

| cmd_type | Name                  | Key fields                                              |
|----------|-----------------------|---------------------------------------------------------|
| 0x01     | SetSwitchPosition     | `gID`, `position` (STRAIGHT \| DIVERGENT)              |
| 0x02     | SetSignalAspect       | `gID`, `aspect` (S1_STOP \| S2_PROCEED \| MS1_STOP \| …) |
| 0x03     | SetDerailerPosition   | `gID`, `position` (LOCKED \| UNLOCKED)                 |
| 0x04     | SetBlockSection       | `gID`, `state` (OPEN \| CLOSED)                        |
| 0x05     | RequestRoute          | `from_signal_gID`, `to_signal_gID`                     |
| 0x06     | CancelRoute           | `route_id`, `force` (bool)                             |
| 0x07     | AcknowledgeAlarm      | `alarm_id`                                             |
| 0x08     | SetBlockDirection     | `block_section_gID`, `operation` (BLW \| BLP \| BLO \| BLZ \| BLAI \| BLA \| OPS) — ML8/SHL-12 only |
| 0x09     | InitAxleCounterReset  | `block_section_gID` — SLI procedure, ML8 only          |
| 0x0A     | ResetAxleCounter      | `block_section_gID` — SLK procedure, ML8 only          |
| 0x20     | OperatorCommand       | `target_g_id`, `target_kind`, `command_code` - EbiLock/EbiScreen X4 operator catalog |
| 0x21     | Ml8Command            | `target_g_id`, `target_kind`, `command_code` - ESTW ML8 operator catalog |

`RequestRoute` is the primary high-level command. The engine resolves required switch positions and signal aspects internally. Low-level commands (`SetSwitchPosition`, `SetSignalAspect`) remain available for manual override.

Commands 0x08–0x0A and 0x21 are only accepted when the scenario's `control_system` is `"estw_ml8"`.  An EbiLock X4 session will reject them with NAK 0x07 (`UNSUPPORTED`).  See [doc 17](17-control-system-interface.md) for the SHL-12 state machine and [doc 19](19-ml8-description.md) for the ML8 operator catalog.

### Command acknowledgement

```
Server → Client (0x11 COMMAND_ACK):  req_seq_id, event_id
Server → Client (0x12 COMMAND_NAK):  req_seq_id, reason_code, reason_text
```

Reason codes (uint8):

```
0x00  UNSPECIFIED    — fallback/default value; should not be emitted intentionally
0x01  NOT_FOUND      — gID not found in current topology (previously UNKNOWN_OBJECT)
0x02  SAFETY_BLOCK   — interlocking rules prevent execution
0x03  INVALID_STATE  — object in a state that rejects this command
0x04  ROUTE_LOCKED   — a route currently locks the device
0x05  NO_PATH        — BFS found no route between entry and exit signals
0x06  SWITCH_MOVING  — switch is in MOVING state; cannot command it
0x07  UNSUPPORTED    — command type not handled by the active SRK system
0x08  UNAUTHORIZED   — client does not own the posterunek
0x09  SESSION_PAUSED — session is not accepting commands
```

> **Note:** codes 0x01–0x07 are defined and checked by the SRK libraries (`libsrk_ebilock`, `libsrk_ml8`).  Codes 0x08–0x09 are checked earlier, on WORK_POOL, by `OwnershipGuard` and session state before the command reaches the engine.

---

## Domain events (S → C broadcast, msg_type = 0x20)

The `DOMAIN_EVENT` payload begins with a 1-byte `event_type` field, a 4-byte `event_id` (server-assigned, monotonically increasing), an 8-byte `timestamp_us` (microseconds since session epoch), then the FlatBuffers body.

| event_type | Name                         | Key fields                                                        |
|------------|------------------------------|-------------------------------------------------------------------|
| 0x01       | SwitchPositionChanged        | `gID`, `new_position`, `cause` (cmd \| auto)                     |
| 0x02       | SwitchOccupancyChanged       | `gID`, `occupied`, `axle_count`, `train_gID`                     |
| 0x03       | SignalAspectChanged          | `gID`, `new_aspect`, `cause`                                     |
| 0x04       | TrackSectionOccupancyChanged | `gID`, `occupied`, `axle_count`, `train_gID`                     |
| 0x05       | DerailerPositionChanged      | `gID`, `new_position`, `cause`                                   |
| 0x06       | BlockSectionStateChanged     | `gID`, `new_state` (OPEN \| CLOSED)                              |
| 0x10       | BlockDirectionStateChanged   | `gID`, `new_direction`, `requires_neighbor_confirmation` (bool)  |
| 0x07       | RouteSet                     | `route_id`, `from_gID`, `to_gID`, `section_ids[]`                |
| 0x08       | RouteReleased                | `route_id`, `reason`                                             |
| 0x09       | TrainMovement                | `train_gID`, `section_gID`, `direction`, `speed_kmh`             |
| 0x0A       | AlarmRaised                  | `alarm_id`, `alarm_type`, `object_gID`, `message`                |
| 0x0B       | AlarmCleared                 | `alarm_id`                                                       |
| 0x0C       | PosterunekOwnershipChanged   | `posterunek_id`, `station_sID`, `new_owner_client_id`            |
| 0x0D       | SessionStateChanged          | `new_state` (STARTED \| PAUSED \| RESUMED \| ENDED)              |
| 0x0E       | TrainComposed                | `train_gID`, `vehicle_gIDs[]`, `total_length_m`, `total_axles`   |
| 0x0F       | TrainDecomposed              | `train_gID`, `reason`                                            |
| 0x11       | OperatorCommandStateChanged  | `g_id`, `target_kind`, `command_code: OperatorCommandCode`, `active` |
| 0x12       | Ml8CommandStateChanged       | `g_id`, `target_kind`, `command_code: Ml8CommandCode`, `active`  |

Events 0x11 and 0x12 are emitted whenever an operator-command runtime flag changes on a device (signal, switch, derailer, track section, block section).  `active = true` means the command is in effect; `active = false` means it was cleared.  These are the wire companions of the `OperatorCommandRuntimeState` field carried in each snapshot device record (see snapshot section below).

Events are **broadcast to all connected clients**. The client filters display by its assigned stations.

---

## Snapshot structure (msg_type = 0x31)

A snapshot is the complete current engine state serialized as a FlatBuffers `Snapshot` table, split into chunks of ≤ 64 kB each.

Snapshots are produced from `EngineSnapshot` — an immutable deep copy of all world-state maps published by the ENGINE after each tick via `AtomicSnapshot`.  Any thread (IO_POOL, WORK_POOL) may call `AtomicSnapshot::load()` to obtain the most recent snapshot without blocking the ENGINE.  See [doc 17](17-control-system-interface.md) for the `AtomicSnapshot` API.

Fields:

```
Snapshot {
  schema_version:   uint32
  session_id:       string
  seq_cursor:       uint32          // seq_id of last event included
  timestamp_us:     uint64
  switches:         [SwitchState]
  track_sections:   [TrackSectionState]
  signals:          [SignalState]
  derailers:        [DerailerState]
  block_sections:   [BlockSectionState]  // includes direction (BlockDirectionState) for SHL-12 blocks
  active_routes:    [RouteState]
  active_alarms:    [AlarmState]
  posterunek_assignments: [PosterunekOwnership]
  trains:                 [TrainState]
}
```

Each of `SwitchState`, `TrackSectionState`, `SignalState`, `DerailerState`, and `BlockSectionSnapshotState` contains an `operator_state: OperatorCommandRuntimeState` field that reflects any currently active EbiLock operator command (or ML8 command) on that device:

```
OperatorCommandRuntimeState {
  active_operator_command: OperatorCommandCode  // present when an OperatorCommand flag is set
  active_ml8_command:      Ml8CommandCode       // present when an Ml8Command flag is set
}
```

The `operator_state` field is the snapshot equivalent of events 0x11 and 0x12.  On reconnect, a client receiving the snapshot does not need to replay prior events to reconstruct current command-flag state.

For Gdynia Główna Osobowa scale (~60 switches, ~120 sections, ~80 signals) estimated snapshot size is **< 50 kB** uncompressed — single chunk in almost all cases.

---

## Multi-operator model (posterunki)

A **station** (`sID`, e.g., `GGO`) may contain multiple **posterunki** (sub-posts, e.g., `GGO_nastawnia_A`, `GGO_nastawnia_B`). Each posterunek defines the scope of objects (switches, signals) that the assigned operator may command. Multiple operators can work the same station simultaneously, each controlling their posterunek.

`HANDSHAKE_ACK` returns `assigned_posterunki[]`, where each entry is:

```
PosterunekAssignment {
  posterunek_id:  string   // e.g. "GGO_nastawnia_A"
  station_sID:   string   // parent station
  display_name:  string
  object_gIDs:   [string] // objects in scope for this sub-post
}
```

For MVP with simple scenarios, a station has exactly one posterunek and the model degenerates to the original per-station ownership. The data model is the same — no special-casing needed.

---

## Ownership protocol (messages 0x50 / 0x51)

```
Client                          Server
  │── TAKEOVER_REQUEST ────────▶ │  posterunek_id, station_sID, reason_text
  │◀─ TAKEOVER_RESPONSE ────────  │  granted: bool, posterunek_id, [reject_reason]
  │                               │  if granted → DOMAIN_EVENT PosterunekOwnershipChanged
  │                               │       broadcast to all clients
```

The server grants the request if: the requesting client is connected, the posterunek is currently unowned or owned by the requesting client, and no active safety-critical route covers objects in that posterunek. Otherwise it denies with a `reject_reason` string.

---

## Player communication

### Chat (msg_type = 0x60)

The client sends `CHAT_MESSAGE` to the server; the server routes it to the target and stores it in `session.chat_log`.

```
ChatMessage {
  target:       ChatTarget   // BROADCAST | STATION | PLAYER
  target_id:    string       // station sID or client_id when target ≠ BROADCAST
  text:         string       // max 500 characters; server truncates silently
  sender_id:    string       // filled in by server on delivery
  timestamp_us: uint64
}
```

Chat messages are delivered over the existing TCP game-state socket — they are low-frequency and must be reliably ordered (e.g., emergency coordination).

### Voice (post-MVP) — architecture decision

**Voice audio MUST NOT travel over the game-state TCP socket.** Reasons:
- TCP head-of-line blocking: a single lost packet stalls all subsequent audio frames.
- Voice tolerates loss (playing 20 ms of silence is acceptable); state events must never be dropped.
- At 20 ms Opus frames (~60 bytes), TCP framing overhead (16 bytes) is 27% — acceptable for chat, wasteful for audio.

**Reserved design:**
- `0x70 VOICE_CHAN_JOIN` / `0x71 VOICE_CHAN_LEAVE` travel over TCP (signaling only; low-frequency).
- `0x72 VOICE_CHAN_STATE` broadcast: who is in which channel.
- Audio stream uses a **separate UDP socket** on a different port, with Opus-encoded RTP or a lightweight custom framing (32-byte header: session_id, channel_id, seq, timestamp, payload). This channel is optional — sessions without voice still work.
- Concrete implementation deferred to post-MVP. msg_type range `0x70–0x7F` is reserved for voice.

### Future: external access

The frame design is transport-agnostic. For external (internet-facing) access:
- Wrap the TCP socket in **TLS 1.3** — the 16-byte frame is unchanged; TLS sits below.
- `auth_token` in `HANDSHAKE` is the authentication hook — replace the placeholder with a signed JWT or session token issued by a future auth service.
- The `magic` field (0x5352) rejects non-SRK connections before any crypto overhead.
- UDP voice channel wraps in **DTLS 1.3** (standard for RTP security).

No changes to the framing protocol are required for this path — the hooks are already present.

---

## Sequencing and error handling rules

1. `seq_id` is per-connection, starts at 1, wraps at `UINT32_MAX`.
2. The server echoes `req_seq_id` in every `COMMAND_ACK` and `COMMAND_NAK`.
3. The server's `DOMAIN_EVENT` stream uses a separate monotonic `event_id` counter, independent of `seq_id`.
4. A client that receives a frame with a bad CRC MUST close the connection and reconnect.
5. A client that receives `payload_len > 65535` MUST close the connection (protocol violation).
6. The server does not queue commands from a client that has not completed the handshake.
7. Command ordering: the server processes commands from a single client in `seq_id` order. Commands from different clients are serialized by the engine's command queue (FIFO, no priority for MVP).

---

## Frame design validation

The additions in this revision (chat, voice signaling, multi-operator, vehicle model) were evaluated against the 16-byte frame:

| Concern | Assessment |
|---|---|
| `msg_type` capacity | 1 byte = 256 slots. Allocated: 18. Range 0x80–0xFF free. ✓ |
| `payload_len` size | uint32 (4 GB theoretical). MVP cap 65 535 bytes. Largest expected payload: snapshot ~50 kB. ✓ |
| `seq_id` wrap | At 1 000 msg/s wraps in ~49 days. Acceptable. ✓ |
| CRC-32 | Detects transmission errors on trusted internal network. For external access TLS provides stronger integrity; CRC still useful as a quick framing sanity check. ✓ |
| Voice audio | Does **not** use this frame — separate UDP channel. Frame overhead (16 B) is irrelevant for audio. ✓ |
| Train snapshot growth | 50 vehicles × ~120 bytes = 6 kB per train; 10 trains = 60 kB — fits in one snapshot chunk. ✓ |
| Multi-operator | Changes payload field names only (`assigned_posterunki[]`). Frame unchanged. ✓ |

**Conclusion: the 16-byte frame is sufficient for all planned extensions. No changes required.**

---

## File layout in repository

```
proto/
  session.fbs     ← handshake, heartbeat, snapshot request, session notice
  commands.fbs
  events.fbs
  snapshot.fbs
  ownership.fbs
  chat.fbs
  voice.fbs
  common.fbs      ← shared enums (aspect, position, alarm_type, chat_target, …)
```

The generated C++ headers are built by CMake target `generate_proto_headers` into `build/generated/proto` (they are not committed to the repository). Schema changes are validated by the `tests_proto` CTest suite. Schema evolution follows a two-phase process: add fields with `= null` defaults first; remove deprecated fields only after both client and server have shipped the new version.

---

## Open questions

- Q-COM-1: Should `DOMAIN_EVENT` carry a diff-only payload (only changed fields) or always the full object state? Full state is simpler; diff is smaller. At Gdynia scale a full-state event for a switch is ~40 bytes — full state is fine for MVP.
- Q-COM-2: ~~Per-station event subscription vs. broadcast?~~ **Resolved (this revision):** full broadcast for MVP; client filters by assigned posterunki. Voice channel architecture (separate UDP) makes selective game-state broadcast unnecessary.
- Q-COM-3: Is a `PING`/`PONG` latency probe needed separately from `HEARTBEAT`, or does the heartbeat timestamp difference suffice for N-001 monitoring?
- Q-COM-4: Signal aspect enum: define a closed list per project now, or leave as a string for flexibility? Closed enum preferred for type safety; list needs to be agreed with the domain model.
- Q-COM-5: Posterunek scope definition — should `object_gIDs[]` be an explicit allowlist per posterunek in the station config, or derived automatically from topology (e.g., all objects within a named zone polygon)?
- Q-COM-6: ~~Chat retention — should chat messages be stored in the event log (same table as domain events) or in a separate chat log?~~ **Resolved:** use separate `session.chat_log` table to keep domain event replay independent from free-text communication payloads.
