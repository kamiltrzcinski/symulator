# Dedicated server sizing baseline

## Purpose

Provide a baseline for selecting a server machine. Every number here is derived from a first-principles load analysis for the chosen stack (C/C++ engine, PostgreSQL, WebSocket or gRPC transport), not from generic rules of thumb.

## MVP load profile

Resolves P0 backlog item "Define MVP load profile".

### Scenario: full Trójmiasto corridor (9 stations)

| Parameter | MVP minimum | Full Trójmiasto | Notes |
|---|---|---|---|
| Concurrent sessions | 1 | 1 | One shared simulation |
| Stations in session | 1 reference | 9 | Gdynia Chylonia → Gdańsk Orunia |
| Posterunki per station (avg) | 1 | 2–3 | Nastawnia A, B, etc. |
| Operator clients (signaling) | 2 | 18–27 | One per posterunek |
| Operator clients (EDR) | 1 | 9 | One per station |
| **Total clients (peak)** | **3** | **~36** | |
| Active trains in session | 2–5 | 20–30 | Rush-hour peak |
| Engine tick rate | 10 Hz | 20 Hz | 50 ms cycle |
| DOMAIN_EVENTs per second (steady) | 5–10 | 20–50 | Occupancy + signal changes |
| DOMAIN_EVENTs per second (burst) | 30 | 100 | Train entering busy throat |
| Commands per second (steady) | 1–2 | 5–15 | All operators combined |
| Avg event payload (FlatBuffers) | 40–120 bytes | 40–120 bytes | Per-object full state |
| Snapshot size | ~10 kB | ~50 kB | Single chunk |
| EDR rows per session | ~10 | ~1 020 | From `11-database-model.md` |

### Broadcast fanout (worst case)

At 36 clients and 100 events/s: **3 600 frame writes/s**, ~120 bytes each = **3.4 Mbps outbound**.
Still well within the 50 Mbps recommended uplink. PostgreSQL event log at 100 writes/s is trivial.

The session server must handle 36 simultaneous TCP connections in a single select/epoll loop — straightforward for a C++ server.

### Design target

The architecture is sized for full Trójmiasto from the start. MVP will be validated with 2–3 clients; the server hardware (recommended 4 vCPU / 8 GB) handles the full scenario without changes.

---

## Load assumptions used in sizing calculations

| Parameter | Value | Rationale |
|---|---|---|
| Concurrent sessions | 1 | Single shared session |
| Operator clients total | 36 | Full Trójmiasto peak (see table above) |
| Engine tick rate | 20 Hz | 50 ms per cycle, enough for signaling state changes |
| State pushes per second (per session) | 100 | Burst during train movement through complex throat |
| Average event payload | 120 bytes | FlatBuffers binary (replaces JSON estimate) |
| Average command payload | 80 bytes | FlatBuffers binary |
| Peak operator commands per second | 2 per client | Realistic burst for an active operator |
| TCP framing overhead factor | 1.1 | Binary framing (16-byte header); much lower than JSON/WebSocket |

## What actually loads the machine

### CPU

Three processes share the machine:

**1. Simulation engine (C++)**
- Runs a fixed 20 Hz tick loop — one cycle every 50 ms.
- Each tick: traverse the interlocking graph, evaluate rule conditions, advance train occupancy, emit changed-state events.
- For a 50 to 150 element station topology (switches, track sections, signals) this is microseconds of work per tick in compiled C++. The graph is small and fits entirely in L2 cache.
- Burst cost: command validation checks the full topology for safety conflicts. Still sub-millisecond at this scale.
- **Estimated CPU share: 2–5% of one modern core at steady state.**

**2. Session server process (C++)**
- Accepts connections, deserializes incoming commands, routes to engine, serializes and fans out state events to all clients.
- Fanout at peak: 8 clients × 20 pushes/sec = 160 serializations/sec at ~500 bytes each. This is trivial for a C++ server; comparable workloads are handled at millions of messages per second in production systems.
- Connection handling for 8–16 WebSocket clients in C++ consumes negligible CPU.
- **Estimated CPU share: 3–8% of one core at steady state.**

**3. PostgreSQL**
- Event log: inserts at roughly the same rate as state changes — 20 to 60 writes/sec. PostgreSQL sustains thousands of inserts/sec on any modern SSD with default settings.
- Configuration reads and snapshot queries happen only at session start or reconnect — infrequent.
- **Estimated CPU share: 2–5% of one core at steady state.**

**Combined realistic load with 2 sessions and 8 clients: 10–25% of a single modern core.**

The minimum of 2 cores is not about raw throughput — it is about isolation: the engine tick loop should never compete with OS scheduling or network I/O on the same core. The recommended 4 cores adds room for the AI worker, profiling, and OS background tasks without any contention.

### RAM

| Component | Estimate | Reasoning |
|---|---|---|
| Simulation engine | < 20 MB | 50–150 node graph with state, ring buffer for events |
| Session server | 10–50 MB | ~200–500 KB per client connection + OS socket buffers |
| PostgreSQL | 300–600 MB | 256 MB shared_buffers minimum; small working set stays cached |
| OS + runtime | 400–700 MB | Linux base system, libc, logging daemon |
| **Total typical** | **800 MB – 1.4 GB** | |

**Minimum 4 GB** provides 2.5–3× headroom over typical consumption. The extra space is used by OS page cache, which PostgreSQL relies on for read performance, and by the pre-loaded AI worker when it is added.

**Recommended 8 GB** allows aggressive PostgreSQL tuning (`shared_buffers` = 2 GB, `work_mem` = 64 MB) and comfortable operation of dev tools or a monitoring agent on the same machine.

### Disk

| Item | Size | Notes |
|---|---|---|
| OS + packages | 5–10 GB | |
| Server binaries and configs | < 100 MB | C++ stripped binaries are small |
| PostgreSQL event log | ~50 MB/1000 sessions | 100 events × 500 bytes per session |
| Session snapshots | ~1 MB each | Full state serialization |
| Logs and core dumps | 2–5 GB | Needs log rotation policy |
| **First-year total (no pruning)** | **< 20 GB** | |

**Minimum 20 GB SSD** is sufficient with a retention policy in place.

**Recommended 40 GB NVMe** — the NVMe distinction matters specifically for PostgreSQL: WAL (Write-Ahead Log) writes are synchronous by default. NVMe write latency (0.1–0.2 ms) vs. SATA SSD (0.5–1 ms) is directly visible in event log insert throughput under burst load.

SD card is unacceptable for the PostgreSQL data directory due to sequential write latency, write endurance limits, and fsync behavior.

### Network

**Outbound bandwidth (state fan-out):**

$$BW_{out} = clients \times updates/sec \times payload \times 8 \times 1.4$$

$$8 \times 20 \times 500 \times 8 \times 1.4 = 896{,}000\ bps \approx 0.9\ Mbps$$

**Inbound bandwidth (commands):**

$$BW_{in} = 8 \times 2 \times 200 \times 8 \times 1.4 \approx 0.04\ Mbps$$

**Total under normal load: under 1 Mbps.** Raw bandwidth is not the constraint.

The real constraints are link quality:

- **RTT** adds directly to command acknowledge round-trip time. A 100 ms RTT alone consumes 83% of the 120 ms p95 SLO budget before any processing.
- **Jitter** causes irregular event delivery — the UI appears to stutter even when the server is healthy.
- **Packet loss above 1%** causes TCP retransmits that push p95 latency well above thresholds and make the simulator feel unresponsive.

The minimum throughput of 20 Mbps is set to be 20× the expected peak load — not because the traffic demands it, but to avoid congestion on a shared uplink during simultaneous snapshot transfers on reconnect.

## Machine specifications

### Minimum viable

- CPU: 2 vCPU, ~3 GHz class (x86_64 or ARM64)
- RAM: 4 GB
- Disk: 20 GB SSD
- Throughput: 20 Mbps symmetric
- Link quality: RTT < 80 ms, jitter < 20 ms, packet loss < 1%

### Recommended starter

- CPU: 4 vCPU
- RAM: 8 GB
- Disk: 40 GB NVMe SSD
- Throughput: 50 Mbps symmetric
- Link quality: RTT < 60 ms, jitter < 10 ms, packet loss < 0.5%

### Growth tier

Trigger when sustained CPU > 70%, > 4 concurrent sessions, or AI worker is colocated:

- CPU: 8 vCPU
- RAM: 16 GB
- Disk: 80+ GB NVMe SSD
- Throughput: 100 Mbps symmetric

## Raspberry Pi 5 (16 GB) assessment

**Hardware profile:**
- CPU: 4× ARM Cortex-A76 @ 2.4 GHz (ARM64)
- RAM: 16 GB LPDDR4X
- Network: built-in Gigabit Ethernet
- Storage: microSD slot — must NOT be used for the database data directory

**Against requirements:**

| Dimension | Requirement | RPi5 16 GB | Assessment |
|---|---|---|---|
| CPU | 4 vCPU recommended | 4× A76 @ 2.4 GHz | Meets. A76 has strong IPC; C++ compiled with `-O2 -march=armv8.2-a` runs efficiently |
| RAM | 8 GB recommended | 16 GB | Exceeds. Room for AI worker and aggressive PostgreSQL caching |
| Storage | 40 GB NVMe SSD | USB 3 NVMe enclosure or NVMe HAT required | Meets with external NVMe; unacceptable on SD card |
| Network | 50 Mbps, jitter < 10 ms | Gigabit + fiber | Exceeds completely; fiber jitter is typically 1–3 ms within the same country |
| Power | n/a | ~8–12 W sustained | Efficient for 24/7 operation |

**Risk factors to address before relying on it:**

1. **Thermal throttling** — under sustained load the A76 will reduce clock speed without active cooling. A heatsink and fan are required; passively cooled cases are insufficient for 24/7 server duty.
2. **Dynamic IP** — residential fiber typically reassigns the IP periodically. A DDNS service (e.g. DuckDNS, Cloudflare DDNS) is needed unless the ISP provides a static address.
3. **Single point of hardware failure** — one board, no redundancy. Acceptable for a personal project; document the backup/restore procedure so a board replacement does not lose session history.

**Verdict:** RPi5 16 GB + fiber + USB NVMe meets and exceeds the recommended specification tier for the entire foreseeable project scale including the AI worker phase. It is the practical long-term choice.

## Raspberry Pi 5 (8 GB) assessment

**Hardware profile:**
- CPU: 4× ARM Cortex-A76 @ 2.4 GHz (ARM64) — identical to RPi5 16 GB
- RAM: 8 GB LPDDR4X
- Network: built-in Gigabit Ethernet
- Storage: PCIe 2.0 lane via M.2 HAT — NVMe directly on PCIe, not through USB

**Against requirements:**

| Dimension | Requirement | RPi5 8 GB | Assessment |
|---|---|---|---|
| CPU | 4 vCPU recommended | 4× A76 @ 2.4 GHz | Meets fully — identical CPU to 16 GB variant |
| RAM | 8 GB recommended | 8 GB LPDDR4X | Meets exactly; no headroom for a large AI model |
| Storage | 40 GB NVMe SSD | PCIe NVMe via M.2 HAT | Meets and exceeds USB path — PCIe NVMe latency ~0.05–0.1 ms |
| Network | 50 Mbps, jitter < 10 ms | Gigabit Ethernet + fiber | Exceeds completely |
| Power | n/a | ~8–12 W sustained | Same as 16 GB variant |

**Where 8 GB becomes the constraint:**

The CPU is no different from the 16 GB model — so all CPU-based session capacity numbers are identical. RAM is the only dimension that changes.

| Component at runtime | Size |
|---|---|
| OS + runtime | 400–700 MB |
| PostgreSQL (shared_buffers 1 GB) | ~1.2 GB |
| Engine + server base | ~70 MB |
| Available for sessions + AI worker | ~6.0 GB |

With no AI worker, the session ceiling is the same as RPi5 16 GB — RAM is not limiting at this scale.

With an AI worker, the margin shrinks:

| AI model size | RAM left for sessions | Safe concurrent sessions |
|---|---|---|
| < 1 GB (small inference model) | ~5 GB | 8–10 |
| ~2 GB | ~4 GB | 6–8 |
| ~4 GB | ~2 GB | 2–3 |
| > 6 GB | < 0 GB | Not viable on 8 GB |

For the project scope (2 players, small signaling model, lightweight AI), 8 GB is sufficient. The 16 GB variant becomes meaningful only when the AI model grows beyond ~2–3 GB or when you want to run model training on the same machine.

**RPi5 8 GB vs RPi5 16 GB — is 80 PLN worth it?**

| | RPi5 8 GB | RPi5 16 GB |
|---|---|---|
| CPU | Identical | Identical |
| PCIe NVMe | Yes | Yes |
| Sessions without AI | Identical ceiling | Identical ceiling |
| Sessions with small AI model (< 2 GB) | Comfortable | Comfortable |
| Sessions with large AI model (> 4 GB) | Not viable | Viable |
| Model training on the same machine | Not viable | Possible (limited) |
| Price delta | — | +80 PLN |

**Verdict:** For the current project scope the RPi5 8 GB is sufficient and the 80 PLN saving is reasonable. If there is any realistic chance of running a larger AI model or training experiments on the same board, pay the 80 PLN — the CPU and PCIe storage are already identical and RAM is the only thing you are buying.

## Raspberry Pi 4 Model B (8 GB) assessment

**Hardware profile:**
- CPU: 4× ARM Cortex-A72 @ 1.8 GHz (ARM64)
- RAM: 8 GB LPDDR4
- Network: built-in Gigabit Ethernet + 802.11ac WiFi (dual-band)
- Storage: microSD slot — must NOT be used for the database data directory; NVMe via USB 3.0 enclosure

**Against requirements:**

| Dimension | Requirement | RPi4 8 GB | Assessment |
|---|---|---|---|
| CPU | 4 vCPU recommended | 4× A72 @ 1.8 GHz | Borderline. A72 IPC is ~35% lower than A76; effective throughput is roughly 60% of RPi5 per clock × 0.75 on clock = ~45% of RPi5 total CPU capacity |
| RAM | 8 GB recommended | 8 GB LPDDR4 | Meets exactly, no headroom for AI worker |
| Storage | 40 GB NVMe SSD | USB 3.0 NVMe enclosure | Meets. USB 3.0 NVMe delivers ~300–400 MB/s — well above PostgreSQL WAL requirements |
| Network | 50 Mbps, jitter < 10 ms | Gigabit Ethernet | Meets on wired. WiFi is unsuitable for server duty — see below |
| Power | n/a | ~6–8 W sustained | Slightly lower than RPi5 |

**WiFi: do not use for the server network interface.**

WiFi introduces jitter of 5–30 ms and variable throughput due to channel contention, retransmits, and power-save modes. This directly violates the RTT and jitter requirements. The Gigabit Ethernet port must be used, even if that requires running a cable.

**Session capacity analysis:**

Binding resource at MVP scale is CPU burst headroom, not average load (the average is tiny — see CPU section above). The practical limit is about keeping p95 loop time under 20 ms when multiple sessions spike simultaneously.

Per-session resource consumption estimate (4 clients, 20 Hz tick, C++ engine):

| Resource | Per session | Basis |
|---|---|---|
| CPU (steady state) | ~4–8% of one A72 core | Engine tick ~1% + server fanout ~2% + PostgreSQL ~2%; A72 is slower than the baseline used earlier |
| RAM increment | ~60–90 MB | Engine graph ~20 MB + 4 client buffers ~20 MB + PostgreSQL working set ~40 MB |

With 4 cores at 65% sustained target and OS overhead taking ~1 core equivalent:

    Available CPU budget: 3 cores × 65% = 1.95 core-equivalents
    Per session CPU cost: ~6% of one A72 core = 0.06 cores
    Theoretical session ceiling (CPU): 1.95 / 0.06 = ~32 sessions

With 8 GB RAM and PostgreSQL tuned to 1 GB shared_buffers:

    Available RAM: 8 GB − 0.6 GB (OS) − 1.2 GB (PostgreSQL base) − 0.1 GB (server base) = ~6.1 GB
    Per session RAM increment: ~75 MB
    Theoretical session ceiling (RAM): 6100 / 75 = ~81 sessions

In practice neither ceiling is the real constraint at project scale. The binding factor is the AI worker: it is CPU-heavy and will run on the same machine. With the AI worker active the CPU budget shrinks significantly.

**Realistic safe session estimates:**

| Scenario | Safe concurrent sessions |
|---|---|
| No AI worker, engine + server + DB only | 8–12 |
| With AI worker active (single model inference thread) | 4–6 |
| With AI worker and active profiling/dev tools | 2–4 |

For the project's actual use case (2 players, 1–2 stations) the RPi4 8 GB handles it comfortably without the AI worker. With the AI worker added it remains viable but will have less headroom than RPi5.

**Risk factors specific to RPi4:**

1. **Thermal throttling** — A72 under sustained load requires active cooling. More prone to throttle than A76 due to older process node (28nm vs 16nm on A76).
2. **No PCIe lane** — NVMe access goes through USB 3.0 which adds ~0.1–0.2 ms latency vs direct PCIe on RPi5. Still acceptable for PostgreSQL WAL.
3. **RAM ceiling with AI worker** — 8 GB leaves no spare capacity once PostgreSQL, the engine, and an AI model are loaded simultaneously. If the model is larger than ~2 GB, RAM becomes the hard limit.
4. **Same dynamic IP and single hardware failure risks as RPi5.**

**Verdict:** RPi4 8 GB + wired Gigabit + USB NVMe meets the minimum specification and handles 1–2 sessions well. It is a valid starting point if RPi5 is unavailable. The practical ceiling before it becomes uncomfortable is around 4–6 sessions with an AI worker running. Upgrade path to RPi5 (or equivalent) is straightforward — same OS, same binaries compiled for ARM64, different board.

**RPi4 vs RPi5 comparison summary:**

| | RPi4 8 GB | RPi5 16 GB |
|---|---|---|
| CPU per session headroom | Sufficient for 4–6 sessions with AI | Comfortable for 8–12 sessions with AI |
| RAM ceiling | Tight with AI worker | Generous |
| NVMe path | USB 3.0 (adequate) | PCIe (faster, lower latency) |
| Recommended use | Good start, upgrade later | Long-term choice |

## Validation targets

1. p95 command acknowledge latency < 120 ms.
2. p95 server update loop time < 20 ms.
3. Average CPU < 65% during peak load test.
4. Average RAM < 70% during peak load test.

## Benchmark plan

1. Prepare a deterministic scenario with a fixed command replay script (no human input).
2. Replay with 2, 4, and 8 simulated clients running in parallel.
3. Measure p50/p95 for command latency, loop time, CPU, and RAM.
4. Simulate 1% packet loss with `tc netem` to observe TCP retransmit behavior on latency metrics.
5. Run the full test on the minimum spec and on the recommended spec.
6. Accept the lower tier only if all targets pass with at least 30% CPU and RAM headroom remaining.
