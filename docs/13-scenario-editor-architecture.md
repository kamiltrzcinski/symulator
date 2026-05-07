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

## `libtrackview` — Shared Track Rendering Library

A C++ shared library (`.so` / `.dll`) linked by both the Scenario Editor and the operator client. It is the single source of truth for track schematic visualization — no duplicated rendering code between tools.

### Module structure

```
libtrackview/
  TrackGrid           — in-memory tile model: map<(col, row) → Tile>
  TrackScene          — QGraphicsScene subclass; renders TrackGrid via a TileSet
  StateOverlay        — overlays live runtime state: occupancy, signal aspects, switch positions
  TileSet (abstract)  — defines how each tile type is drawn
    ├── EbiScreenTileSet          — wide flat cells, arrow-head direction markers, coloured occupancy
    └── TechnicalDiagramTileSet   — square cells, full 45° diagonals, monochrome
  TileRegistry        — maps tile type string → connection port geometry (topology; style-independent)
```

`TileRegistry` (topology, which ports connect to which) is separated from `TileSet` (drawing). The same `TrackGrid` renders correctly in both styles without any data conversion.

### Visual style comparison

| Property | EbiScreen style | Technical Diagram style |
|---|---|---|
| Cell ratio | ~3:1 (wide, flat) | 1:1 (square) |
| Diagonal segments | Short corner clips at junctions only | Full 45° runs spanning multiple cells |
| Colour scheme | Occupancy-coloured (green / red / yellow) | Monochrome (white on black) |
| Direction indicators | Arrow-head glyphs on track | None |
| Reference | Jankowo screenshot | Skierniewice screenshot |

The style is a **project-level preference** stored in the `.scendb` project file. It is a pure rendering hint and does not affect the tile data model or the exported JSON bundles.

### Usage in the operator client

The client creates a `LiveTrackScene` (subclass of `TrackScene`) with a `StateOverlay` wired to the server's event stream. No editing tools are exposed. The same `libtrackview` is linked — only the interaction layer differs between editor and client.

---

## Editor 1 — Station Editor

Produces the per-station topology bundle already defined in `08-track-topology-model.md`.

The editor operates **fully offline** — no server connection is required for topology authoring. At project creation the operator selects the visual layout style (`ebi_screen` or `technical_diagram`); this choice is stored in the project and affects only rendering — it does not alter the exported JSON bundle. The style can be changed at any time without data loss.

**Responsibilities:**
- Place and connect `OT` (track sections) and `ZWR` (switches/crossovers) on a tile-based canvas.
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

### Native project format — `.scendb` (SQLite)

The editor's working file is a SQLite database (`.scendb`). The engine never reads `.scendb` directly; it is the editor's internal format. Export to the three-file JSON bundle is a separate, explicit action.

**Rationale over flat JSON:** foreign-key enforcement catches dangling `tile_id` / `from_tile` references at write time; SQL queries support topology analysis during authoring; a single binary file is easy to pass between contributors; `edit_history` provides persistent undo/redo at no extra cost.

```sql
PRAGMA foreign_keys = ON;

CREATE TABLE project_meta (
  key   TEXT PRIMARY KEY,
  value TEXT NOT NULL
  -- keys: station_name, station_sid, area, layout_style, schema_version
);

-- Tile = one cell on the schematic grid
CREATE TABLE tiles (
  id       TEXT    PRIMARY KEY,   -- gID-style, assigned on placement
  col      INTEGER NOT NULL,
  row      INTEGER NOT NULL,
  type     TEXT    NOT NULL,      -- 'straight_h', 'straight_v', 'curve_ne', 'curve_nw',
                                  --  'curve_se', 'curve_sw', 'switch_left', 'switch_right',
                                  --  'diamond', 'slip', 'buffer_stop', 'boundary'
  rotation INTEGER NOT NULL DEFAULT 0,  -- 0 / 90 / 180 / 270
  pID      TEXT,
  UNIQUE(col, row)
);

-- Explicit port-to-port connections between tiles
CREATE TABLE connections (
  id        TEXT PRIMARY KEY,
  from_tile TEXT NOT NULL REFERENCES tiles(id),
  from_port TEXT NOT NULL,   -- 'west', 'east', 'north', 'south', 'trunk', 'straight', 'divergent'
  to_tile   TEXT NOT NULL REFERENCES tiles(id),
  to_port   TEXT NOT NULL,
  UNIQUE(from_tile, from_port)
);

-- Signals, derailers, line blocks, Iz nodes placed on a tile
CREATE TABLE objects (
  id        TEXT PRIMARY KEY,
  tile_id   TEXT NOT NULL REFERENCES tiles(id),
  type      TEXT NOT NULL,   -- 'signal', 'derailer', 'line_block', 'iz'
  pID       TEXT NOT NULL,
  direction TEXT,            -- 'A' / 'B' for signals
  props     TEXT             -- JSON blob for type-specific attributes
);

-- Persistent undo/redo log
CREATE TABLE edit_history (
  seq       INTEGER PRIMARY KEY AUTOINCREMENT,
  timestamp TEXT    NOT NULL,
  action    TEXT    NOT NULL,  -- JSON: {op, target_id, before, after}
  undone    INTEGER NOT NULL DEFAULT 0
);
```

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
| **PLK import** | Triggered from the editor UI; the request is forwarded to the **Session Server**, which calls `IPLKImporter::import(config)` — fetches `GET /api/v1/schedules` for each station SID and maps results to template rows. The editor receives the result and the operator reviews and confirms. The editor itself never contacts PLK directly; a server connection is required for this mode. |
| **Manual edit** | Create / edit template rows directly (useful for fictional or modified timetables). Works fully offline. |

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
