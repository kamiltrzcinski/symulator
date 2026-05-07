# Railway Traffic Control System Manual

# Chapter 1 — General Information

## 1.1 Purpose of the System

The railway traffic control system is designed to:

- supervise train movements,
- control signaling devices,
- manage routes,
- control turnout positions,
- supervise line block systems,
- ensure safe railway operation.

---

## 1.2 Dispatcher Responsibilities

The dispatcher is responsible for:

- safe train movement,
- route management,
- signal operation,
- coordination with neighboring stations,
- emergency handling,
- maintaining operational continuity.

---

## 1.3 Basic Operating Rules

Before issuing any command, the dispatcher must verify:

- track occupancy,
- turnout position,
- signal indications,
- route locking status,
- neighboring station readiness.

---

# Chapter 2 — Route Commands

## 2.1 Route Setting

### Purpose

Used to establish a train route through station tracks and line sections.

### Functions

- turnout positioning,
- turnout locking,
- flank protection,
- signal clearing,
- route locking.

---

## 2.2 Route Cancellation

### Purpose

Used to release or cancel an established route.

### Functions

- route unlocking,
- turnout release,
- signal restoration to stop aspect.

---

## 2.3 Emergency Route Release

### Purpose

Used during failures or abnormal situations to force release locked routes.

### Warning

Emergency release procedures must only be used according to regulations.

---

# Chapter 3 — Signal Commands

## 3.1 Signal Operation

### Purpose

Commands related to signal control.

### Functions

- setting proceed aspect,
- setting stop aspect,
- emergency signal replacement,
- signal cancellation.

---

## 3.2 Signal Safety Rules

Signals may only display proceed aspects when:

- the route is correctly established,
- all turnouts are locked,
- flank protection is active,
- track sections are clear.

---

# Chapter 4 — Turnout Commands

## 4.1 Turnout Control

### Purpose

Commands used to control railway turnouts.

### Functions

- setting normal position,
- setting reverse position,
- turnout locking,
- turnout release.

---

## 4.2 Turnout Safety

Turnouts may not be moved when:

- occupied by rolling stock,
- locked in an active route,
- protected by safety dependencies.

---

# Chapter 5 — SHL-12 Automatic Line Block

## 5.1 General Information

The SHL-12 automatic line block system supervises train movement between stations.

The system:

- controls line direction,
- supervises track occupancy,
- manages train separation,
- supports emergency procedures.

---

## 5.2 General Operating Conditions

SHL-12 commands may only be executed when:

- no route is locked,
- no train movement conflicts exist,
- the block system is operational,
- track conditions allow safe execution.

---

## 5.3 Block Direction States

### Neutral State

No direction assigned.

### Outbound Direction

The station dispatches trains toward the neighboring station.

### Inbound Direction

The station receives trains from the neighboring station.

---

## 5.4 Emergency States

Emergency procedures are used during:

- communication failures,
- occupancy detection failures,
- incorrect direction states,
- axle counter failures,
- abnormal train movements.

---

## 5.5 Axle Counter Supervision

The system supervises:

- occupied sections,
- train axle balancing,
- section clearing,
- reset procedures.

---

## 5.6 Safety Rules

The dispatcher must verify:

- train location,
- section occupancy,
- neighboring station coordination,
- turnout integrity,
- route locking status

before issuing emergency commands.

---

## 5.7 Special Procedures

Special procedures include:

- emergency direction change,
- axle counter reset,
- forced block release,
- special cancellation procedures.

---

## 5.8 Emergency Restrictions

Emergency procedures must only be used:

- during failures,
- according to railway regulations,
- after operational verification.

Improper usage may cause:

- unsafe train movement,
- false occupancy states,
- traffic disruption.

---

## 5.9 Dispatcher Coordination

Neighboring stations must coordinate during:

- block direction changes,
- emergency procedures,
- reset operations,
- abnormal situations.

---

## 5.10 SHL-12 Commands

## 5.10.1 BLW — Set Block Direction to Departure

### Syntax

```text
BLW track
```

### Description

Requests setting the line block direction for train departure toward the neighboring station.

### Example

```text
BLW ZBG_2P
```

---

## 5.10.2 BLP — Permission for Direction Setting

### Syntax

```text
BLP track
```

### Description

Confirms permission to change the block direction requested by `BLW`.

### Example

```text
BLP ZBG_1G
```

---

## 5.10.3 BLO — Cancel Direction Setting Procedure

### Syntax

```text
BLO track
```

### Description

Cancels the block direction setting procedure before confirmation is granted.

### Example

```text
BLO ZBG_2P
```

---

## 5.10.4 BLZ — Release Block Direction

### Syntax

```text
BLZ track
```

### Description

Releases the currently established block direction.

### Example

```text
BLZ ZBG_1G
```

---

## 5.10.5 BLAI — Initialize Emergency Direction Change

### Syntax

```text
BLAI track
```

### Description

Starts the emergency procedure for changing block direction.

### Example

```text
BLAI ZBG_2B
```

---

## 5.10.6 BLA — Emergency Direction Change

### Syntax

```text
BLA track
```

### Description

Performs an emergency reversal of block direction.

### Example

```text
BLA ZBG_1P
```

---

## 5.10.7 OPS — Cancel Special Procedure

### Syntax

```text
OPS track
```

### Description

Cancels a currently initialized or active special procedure.

### Example

```text
OPS ZBG_2B
```

---

## 5.10.17 SLI — Initialize Block Section Reset

### Syntax

```text
SLI section
```

### Description

Initializes the axle counter reset procedure.

### Example

```text
SLI ZBG_it440
```

---

## 5.10.18 SLK — Reset Block Section

### Syntax

```text
SLK section
```

### Description

Resets axle counter occupancy for the selected block section.

### Example

```text
SLK ZBG_it440
```

---
