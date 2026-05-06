# Project documentation

This folder contains the agreed scope and implementation plan.

## Index

- `01-vision-and-scope.md` - what we are building and what belongs to MVP.
- `02-system-requirements.md` - functional and non-functional requirements.
- `03-initial-architecture.md` - component split and communication flow.
- `04-roadmap.md` - delivery stages and completion criteria.
- `05-starter-backlog.md` - first tasks to move into issues.
- `06-server-sizing-baseline.md` - dedicated server hardware/network baseline and validation plan.
- `07-ebiscreen-description.md` - decription about ebiscreen device
- `08-track-topology-model.md` - track section and switch object model; file format decision for station configuration.
- `09-communication-contract.md` - binary frame layout, message type catalog, FlatBuffers serialization decision, command/event/snapshot/ownership payloads.
- `10-vehicle-model.md` - vehicle definition JSON, train composition, axle-counting occupancy model, physics v1/v2.
- `11-database-model.md` - PostgreSQL schema (`fleet` + `session`), EDR live model, retention policy.
- `12-server-api.md` - internal C++ module interface contracts; PLK Open Railway Data API integration for timetable import.
- `13-scenario-editor-architecture.md` - three-editor model (station, route, timetable); `sections.json` data model; `IScenarioLinter` three-layer validation; scenario directory layout; extended area assessment.
## Update rules

- Every major project decision should be captured in the relevant document.
- Requirements use stable IDs, for example `F-001` and `N-001`.
- If a decision changes, update the original section instead of appending conflicting notes.
