# Vision and scope

## Context

The project should deliver a simple but playable railway signaling simulator inspired by Ebilock-like operation:
- sending commands,
- simulating outcomes,
- tracking device states and occupancy,
- executing a planned timetable.

The system is expected to run over a network and connect at least two simulation stations.

## Product goal

Create a stable foundation for shared gameplay and traffic scenario testing, while keeping domain logic independent from any specific UI.

## MVP scope

1. One reference station test map.
2. Signaling state engine (switches, signals, track sections).
3. Track occupancy simulation for train movement.
4. Basic timetable and checkpoint execution.
5. Session server supporting up to 2 operator clients, each managing multiple stations via tabbed panels.
6. Persistent storage for session state and event history.

## Station assignment model

- The scenario covers 5-6 stations total (exact count depends on whether Naterki is included).
- Default assignment: player 1 gets 3 stations, player 2 gets 2-3 stations.
- A single client displays all assigned stations as tabs; the operator switches between panels without launching additional client instances.
- An operator can request permission to take over another station (e.g., boundary/LCS area) and manage it from the same tabbed client.

## Out of MVP scope

- Full compliance with real-world vendor systems.
- Advanced dispatcher-panel graphics.
- Complex AI traffic model with online learning.
- Large multi-station production scenarios.

## User roles

- Signaling operator: sends commands, monitors states and alarms.
- Observer/trainer: watches sessions and reviews history.
- Scenario editor: prepares timetable and station config.

## MVP success criteria

- Two operators can run one session without state divergence.
- Typical command propagation latency client-server-client is <= 100 ms.
- Every command has either a visible simulation effect or a clear rejection reason.
- Session state can be restored from database/event log after restart.
