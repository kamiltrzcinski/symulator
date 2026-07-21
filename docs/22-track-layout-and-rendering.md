# Track layout and rendering

## Decision

Physical topology and presentation geometry are separate contracts:

- `topology.json` describes connections and infrastructure parameters;
- `objects.json` describes signals and other devices;
- `layouts/*.json` describes paths, switch ports and object placement;
- engine state describes occupancy, switch position and signal indication;
- `libtrackview` combines these inputs without owning interlocking rules.

Layouts reference infrastructure UIDs. Topology never references a layout. One
physical station can therefore have EbiScreen, technical and future mechanical
control presentations.

The machine-readable version 1 contract is
[`schemas/track-layout.schema.json`](../schemas/track-layout.schema.json).
Coordinates are logical layout units, not monitor pixels.

## Module boundaries

```text
trackview_core
  layout model
  ILayoutReader <- JsonLayoutReader
  ILayoutValidator <- LayoutValidator
  IAttachmentResolver <- PathAttachmentResolver
  IRenderModelBuilder <- RenderModelBuilder
  IInfrastructureCatalog
  ITrackRuntimeState

trackview_engine_adapter
  EngineInfrastructureCatalogAdapter -> IInfrastructureCatalog
  EngineTrackRuntimeAdapter           -> ITrackRuntimeState

trackview
  ITrackTheme <- EbiScreenTheme
  ISceneRenderer <- QtSceneRenderer
  IImageRenderer <- QtImageRenderer
```

`trackview_core` links neither Qt nor `engine`. The engine dependency points
inward through two adapters. Qt exists only at the final presentation boundary.

## SOLID guarantees

### Single responsibility

- JSON reading, semantic validation, attachment calculation, state composition,
  scene drawing, image drawing, styling and engine translation are separate
  classes and source files.
- `QtImageRenderer` delegates scene creation to `ISceneRenderer`; it does not
  duplicate drawing rules.

### Open/closed

- another input format implements `ILayoutReader`;
- another attachment algorithm implements `IAttachmentResolver`;
- another visual style implements `ITrackTheme`;
- another scene or image backend implements the respective renderer interface;
- another simulation source implements the two data-source interfaces.

The track/switch/signal variant is intentionally the closed vocabulary of the
`track_schematic` bounded context. Mechanical controls belong to a separate
`control_panel` model rather than weakening this contract with unrelated types.

### Liskov substitution

Concrete implementations do not strengthen input preconditions declared by
their interfaces. Tests inject fake catalog, runtime and attachment resolver
implementations to verify substitutability.

### Interface segregation

Validation sees only `IInfrastructureCatalog`. Rendering sees only
`ITrackRuntimeState`. Neither receives the engine's routes, alarms, mutation
methods or session metadata. Scene rendering and image rendering also use
separate interfaces.

### Dependency inversion

High-level validation and render-model construction depend on the interfaces
above. They do not include engine or Qt headers. Engine enums are translated to
stable `trackview` vocabulary by the adapter.

## Layout elements

A track section stores a polyline:

```json
{
  "topology_uid": 1001,
  "kind": "track_section",
  "path": [{"x": 2, "y": 8}, {"x": 20, "y": 8}]
}
```

A switch stores named geometry ports:

```json
{
  "topology_uid": 2001,
  "kind": "switch",
  "ports": {
    "trunk": {"x": 20, "y": 8},
    "straight": {"x": 28, "y": 8},
    "divergent": {"x": 28, "y": 12}
  }
}
```

A signal is attached semantically to a track. `offset` is a normalized distance
from side A or B; `lateral` is perpendicular displacement:

```json
{
  "object_uid": 3001,
  "kind": "signal",
  "attachment": {
    "topology_uid": 1001,
    "side": "A",
    "offset": 0.25,
    "lateral": -2
  },
  "facing": "towards_B"
}
```

## Mechanical signal boxes

A mechanical frame is not an EbiScreen theme. Its levers, keys and block
instruments need a separate `control_panel` layout and command bindings. Their
positions belong to presentation data; permission to move them belongs to the
SRK/interlocking model. This separation is preserved by the current bounded
context.

## Automatic layout

Automatic layout should implement an authoring service which produces a draft
`TrackLayout`. The draft is saved and may be edited. It is not regenerated when
the client opens a station, so algorithm changes cannot rearrange an operator's
panel unexpectedly.

## Verification

Tests cover each responsibility and the complete integration path:

```text
Sopot topology + objects + layout
  -> engine adapters
  -> validation
  -> RenderModel
  -> QGraphicsScene
  -> QImage
```

The Qt test checks visible pixels and infrastructure IDs stored on interactive
graphics items. Exact image hashes are avoided because antialiasing and fonts
vary between platforms.
