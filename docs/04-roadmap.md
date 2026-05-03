# Roadmap

## Stage 0: Discovery and specification (1-2 weeks)

Scope:
- finalize vision and MVP boundaries,
- close core technology decisions,
- define first API contracts.
- decide deployment model and rendering model.
- define dedicated server sizing baseline and validation metrics.

Definition of Done:
- documents 01-03 are aligned,
- risks and dependencies are listed,
- first implementation sprint backlog is ready.
- deployment and rendering decisions are recorded with rationale.
- server baseline requirements and test SLO targets are documented.

## Stage 1: Core engine prototype (2-4 weeks)

Scope:
- station topology model,
- commands and safety validation,
- state and occupancy updates.

Definition of Done:
- one offline reference scenario is runnable,
- critical logic has unit tests,
- map configuration format is stable.

## Stage 2: Server and synchronization (2-3 weeks)

Scope:
- server-authoritative state,
- client join/rejoin with snapshots,
- event log and database persistence.

Definition of Done:
- 2 clients observe the same state,
- reconnect does not break the session,
- session flow can be replayed from logs.

## Stage 3: MVP operator client (2-3 weeks)

Scope:
- basic panel and state view,
- command dispatch,
- alarm and history panel.

Definition of Done:
- an operator can run scenario A-B end to end,
- invalid commands return clear messages,
- UX supports baseline operation without debugger tooling.

## Stage 4: Timetable and gameplay loop (2-3 weeks)

Scope:
- timetable import and execution,
- checkpoint timing validation,
- scoring or session summary report.

Definition of Done:
- full test session can be completed,
- timetable deviations are measured,
- end-of-session report is persisted.
