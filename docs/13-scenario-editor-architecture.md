# Scenario Editor Architecture

## Context

A playable scenario requires three distinct data artefacts:

| Artefact | Produced by | Consumed by |
|---|---|---|
| Station topology bundle (`meta.json`, `topology.json`, `objects.json`) | Station Editor | Engine, Client, Scenario Linter |
| Inter-station section definitions (`sections.json`) | Route Editor | Engine, Scenario Linter |
| Timetable templates | Timetable Editor (PLK import + manual) | EDR, Scenario Linter |

The three editors are independent tools (or tabs within one tool). A **Scenario Linter** (`libscenario_validation`) ties them together and is the gate through which a scenario must pass before the server will load it.

---

## Editor 1 — Station Editor

Produces the per-station topology bundle already defined in `08-track-topology-model.md`.

**Responsibilities:**
- Place and connect `OT` (track sections) and `ZWR` (switches/crossovers) on a canvas.
- Insert `Iz` (axle counter) nodes at section boundaries; assign unique `Iz` IDs.
- Place signals (`SYG`), assign unique IDs, mounting `OT`, direction, and valid aspects.
- Assign `gID` to every object via `generateGID(type, area, pID)`.
- Export: `meta.json`, `topology.json`, `objects.json`.

**Validation (live, Layer 1):**
- Every `OT` edge references valid `Iz` IDs at both ends.
- Every `ZWR` references valid `Iz` IDs for `trunkIzID`, `straightIzID`, `divergentIzID`.
- Every signal has a unique station-scoped ID, a mounting `OT`, and ≥ 1 aspect.
- No dangling references (object referenced but not declared).
- No duplicate `gID` within the station bundle.

---

## Editor 2 — Route Editor

Produces `sections.json` — the inter-station line topology.

### Data model

A route is a named sequence of **line sections** connecting adjacent station pairs.

**`sections.json` (top-level array):**

```json
[
  {
    "section_id": "SEC-TRJ-GOR-SOP",
    "display": { "pl": "Gdynia Orłowo – Sopot", "en": "Gdynia Orłowo – Sopot" },
    "from_station_sid": "GDYNIA_ORLOWO",
    "to_station_sid": "SOPOT",
    "length_m": 4200,
    "max_speed_kmh": 120,
    "tracks": 2,
    "boundary_iz_from": "GDYNIA_ORLOWO.Iz-OUT-N",
    "boundary_iz_to": "SOPOT.Iz-IN-S"
  }
]
```

| Field | Type | Notes |
|---|---|---|
| `section_id` | string | Unique; format `SEC-{AREA}-{FROM_ABBR}-{TO_ABBR}` |
| `display` | i18n map | Human label |
| `from_station_sid` / `to_station_sid` | string | `meta.json` `station_sid` of adjacent stations |
| `length_m` | int | For travel-time plausibility checks |
| `max_speed_kmh` | int | Used in timetable validation |
| `tracks` | int | 1 or 2 (affects scheduling) |
| `boundary_iz_from` | string | `{station_sid}.{Iz_id}` — the last axle counter on the departing side |
| `boundary_iz_to` | string | `{station_sid}.{Iz_id}` — the first axle counter on the arriving side |

**Responsibility of `boundary_iz_*` at runtime:** the engine uses these IDs to determine when a train leaves one station's jurisdiction and enters another's. The interlocking boundary is defined here, not hard-coded.

**Route (`routes.json`, optional grouping):**

```json
[
  {
    "route_id": "RT-TRJ-MAIN",
    "display": { "pl": "Linia Trójmiejska", "en": "Tri-City Line" },
    "ordered_station_sids": [
      "GDYNIA_CHYLONIA", "GDYNIA_POSTOJOWA", "GDYNIA_GLOWNA",
      "GDYNIA_ORLOWO", "SOPOT", "GDANSK_OLIWA",
      "GDANSK_WRZESZCZ", "GDANSK_GLOWNY", "GDANSK_ORUNIA"
    ]
  }
]
```

**Validation (live, Layer 2):**
- Every `from_station_sid` / `to_station_sid` resolves to a loaded station bundle.
- `boundary_iz_from` exists in the `from` station's `topology.json`.
- `boundary_iz_to` exists in the `to` station's `topology.json`.
- No gaps in route: every consecutive pair of stations in `ordered_station_sids` has exactly one `sections.json` entry.
- No orphan section (listed in `sections.json` but not referenced by any route).

---

## Editor 3 — Timetable Editor

Produces / manages `fleet.timetable_templates` rows (as described in `11-database-model.md`).

**Two modes:**

| Mode | Action |
|---|---|
| **PLK import** | Call `IPLKImporter::import(config)` — fetches `GET /api/v1/schedules` for each station SID; maps to template rows. Operator reviews and confirms. |
| **Manual edit** | Create / edit template rows directly (useful for fictional or modified timetables). |

**Validation (live, Layer 3):**
- Arrivals and departures monotonically increasing per train per day (no time-travel).
- Scheduled travel time ≥ `floor(section.length_m / (section.max_speed_kmh / 3.6))` — physically reachable lower bound.
- Every `composition_gid` referenced in a template exists in `fleet.train_definitions`.
- No train spawns mid-route (must originate at a defined terminus or enter from outside the scenario boundary).
- Dwell time at intermediate stations ≥ 30 s (configurable minimum).

---

## Scenario Linter — `libscenario_validation`

A static C++ library. Layers run sequentially; a Layer N failure blocks Layer N+1.

```
Layer 1: Station bundles  (per-station, parallel-safe)
Layer 2: Route topology   (requires Layer 1 OK)
Layer 3: Timetable        (requires Layer 1 + 2 OK)
```

### Diagnostic record

```cpp
struct LintDiagnostic {
    enum class Severity { ERROR, WARNING, INFO };
    std::string    rule_id;       // e.g. "L1-SYG-001"
    Severity       severity;
    std::string    station_sid;   // empty if not station-specific
    std::string    section_id;    // empty if not section-specific
    std::string    train_id;      // empty if not train-specific
    std::string    object_gid;    // empty if not object-specific
    std::string    message;
};
```

### Interface

```cpp
class IScenarioLinter {
public:
    virtual ~IScenarioLinter() = default;

    // Run all three layers; returns all diagnostics found.
    virtual std::vector<LintDiagnostic>
        lint(const ScenarioBundle& bundle) const = 0;

    // Convenience: true iff lint() returns zero ERRORs.
    virtual bool isValid(const ScenarioBundle& bundle) const = 0;
};
```

### `ScenarioBundle` input

```cpp
struct ScenarioBundle {
    std::vector<StationBundle>    stations;   // loaded from editor 1 output
    std::vector<LineSection>      sections;   // loaded from sections.json
    std::vector<Route>            routes;     // loaded from routes.json
    std::vector<TimetableTemplate> templates; // loaded from DB or import
};
```

### Usage contexts

| Context | Action on ERROR |
|---|---|
| Editor (live) | Highlight object in canvas, show message in sidebar |
| CLI `scenario-lint` | Print diagnostics, exit code 1 |
| Server startup | Log diagnostics, refuse to start session |

---

## Scenario directory layout

```
scenarios/
  trojmiasto/
    meta.yaml              # scenario name, version, author
    stations/
      gdynia_chylonia/
        meta.json
        topology.json
        objects.json
      sopot/
        meta.json
        topology.json
        objects.json
      ...
    sections.json
    routes.json
    timetable.sql          # INSERT statements for fleet.timetable_templates (optional export)
```

`meta.yaml` fields: `scenario_id`, `display`, `version`, `author`, `created_at`, `station_sids[]` (ordered list used by the linter to know which station bundles belong to this scenario).

---

## Extended area assessment — Wejherowo → Pruszcz Gdański (13 stations)

MVP is 9 stations (Gdynia Chylonia → Gdańsk Orunia). Two possible extensions:

**North:** Gdynia Chylonia → Rumia → Reda → Wejherowo (+3 stations, +3 sections)  
**South:** Gdańsk Orunia → Pruszcz Gdański (+1 station, +1 section)

| Metric | MVP 9 st. | Extended 13 st. | Notes |
|---|---|---|---|
| Peak clients | ~36 | ~52 | 4 clients/station (2 posterunki × 2 players) |
| Events/s burst | ~100 | ~145 | Linear scale |
| Snapshot size | ~50 kB | ~72 kB | Proportional to object count |
| Broadcast fanout | ~3 600 writes/s ≈ 3.4 Mbps | ~5 200 writes/s ≈ 4.9 Mbps | Well within 50 Mbps uplink |
| EDR rows/session | ~1 020 | ~1 470 | Trivial for PostgreSQL |

**Verdict: server architecture supports 13 stations without changes on 4 vCPU / 8 GB RAM.** The 2 vCPU / 4 GB baseline remains viable with headroom.

**Real constraints are editorial, not technical:**

| Constraint | Notes |
|---|---|
| Wejherowo and Gdynia Chylonia are depot terminals | Complex shunting topology; more switches, more signals, more validation rules to define |
| Staffing | 13 stations = up to 26 posterunki; sessions with fewer than ~7 active players leave most posterunki unowned; covered by 30 s grace revoke, but session design should account for it |
| Editorial throughput | Each station bundle requires a modelled topology; 4 additional stations × topology complexity ≈ significant editor work before linter Layer 1 passes |

---

## Open questions

| ID | Question |
|---|---|
| Q-EDI-1 | Is the editor a standalone desktop app (Qt), a tab inside the client, or a web tool? Standalone is cleanest for Stage 1 (no server running required). |
| Q-EDI-2 | Does the CLI `scenario-lint` ship as part of the server binary or a separate tool? Separate tool is preferable for CI pipelines. |
| Q-EDI-3 | How are PLK `station_code` values mapped to our `station_sid` identifiers? Static lookup table in `IPLKImporter` config vs. auto-detection by name match. |
| Q-EDI-4 | Double-slip and diamond-crossing objects (Q-TOPo-1 / Q-TOPo-2 from `08-track-topology-model.md`) — required for Gdańsk Główny and Wejherowo; must be resolved before those station bundles can pass Layer 1. |
