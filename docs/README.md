# Project documentation

This folder contains the agreed scope and implementation plan.

## Index

- `00-contributing.md` - development environment setup, commit conventions, branch policy.
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
- `14-interlocking-model.md` - safety rule catalog (R1–R10 including SHL-12), route lifecycle, conflict checks, IControlSystem boundary.
- `15-dispatch-forms.md` - S-form exchange model (S2/S24/S25/S26 and related forms).
- `16-implementation-skeleton.md` - implemented domain layer overview; remaining ENGINE integration and broadcast pipeline wiring.
- `17-control-system-interface.md` - IControlSystem / IStateView / DeviceStateChange contract; ControlSystemRegistry; AtomicSnapshot; EbiLock X4 and ML8 SHL-12 implementations.
- `19-ml8-description.md` - ESTW ML8 as a separate control system: command catalog, protocol command, and engine handling notes.
- `18-etcs-rbc-ertms-description.md` - ETCS/RBC supervisory system specification (area, session, commands, text messages, communication supervision).
- `devlog.md` - chronological implementation log and session decisions.
- `20-timetable-operating-days.md` - timetable operating-days legend (`1`=Monday ... `7`=Sunday).
## Update rules

- Every major project decision should be captured in the relevant document.
- Requirements use stable IDs, for example `F-001` and `N-001`.
- If a decision changes, update the original section instead of appending conflicting notes.
