# ETCS/RBC Supervisory System Specification

## Overview

The ETCS/RBC Supervisory System is a centralized supervisory layer intended for operational control and monitoring of Radio Block Centre (`RBC`) areas operating within ERTMS/ETCS infrastructure.

The system is responsible for:

- supervision of RBC operational areas,
- ETCS session management,
- supervision of Movement Authority (`MA`) generation,
- monitoring communication with onboard ETCS equipment,
- supervision of interlocking interfaces,
- handling alarms and operational events,
- operator authorization management,
- execution of operational RBC commands,
- train positioning supervision,
- transmission of text messages to trains,
- presentation of DSAT alarms and diagnostic events.

The supervisory layer cooperates with:

- RBC systems,
- interlocking systems,
- GSM-R infrastructure,
- TMS systems,
- operator workstations,
- diagnostic systems.

Control of operational objects requires obtaining control authority over the selected RBC area.

---

# Object Naming

Objects within the ETCS/RBC supervisory layer follow standardized operational naming conventions.

| Object | Naming Convention |
|---|---|
| RBC | `rbc_X` |
| RBC Area | `area_X` |
| ETCS Session | `ses_X` |
| Movement Authority | `ma_X` |
| Balise Group | `bg_X` |
| LEU | `leu_X` |
| Communication Channel | `com_X` |
| Operator Workstation | `ops_X` |
| System Alarm | `alm_X` |
| Text Message | `msg_X` |

---

# UID Structure

Each object is assigned a UID consisting of:

| Field | Description |
|---|---|
| `gID` | global identifier |
| `pID` | operational identifier |
| `sID` | RBC area identifier |
| `type` | object type |

---

# Operational Naming Rules (pID)

RBC instances:

```text
rbc_<ID>
```

ETCS sessions:

```text
ses_<ID>
```

Movement Authorities:

```text
ma_<ID>
```

Text messages:

```text
msg_<ID>
```

Alarms:

```text
alm_<ID>
```

Other objects:

| Object | Format |
|---|---|
| Balise Group | `bg_<ID>` |
| LEU | `leu_<ID>` |
| Communication Channel | `com_<ID>` |
| Operator Workstation | `ops_<ID>` |

---

# Global Identifier (gID)

Format:

```text
TYPE-AREA-pID-UUID
```

Example:

```text
RBC-WARSAW-rbc_central-0000001
```

| Field | Description |
|---|---|
| `TYPE` | object type |
| `AREA` | supervision area |
| `pID` | operational identifier |
| `UUID` | globally unique identifier |

---

# ID Generation Function

```cpp
string generateGID(string type, string area, string pID)
{
    string idNumber =
        padLeft(to_string(globalCounter), 7, '0');

    string gID =
        type + "-" +
        area + "-" +
        pID + "-" +
        idNumber;

    globalCounter++;

    return gID;
}
```

---

# RBC Topology Model

## RBC Operational Area

An RBC area represents a logical supervision domain responsible for:

- ETCS supervision,
- Movement Authority generation,
- maintaining ETCS sessions,
- communication with interlocking systems,
- GSM-R communication handling.

The operational area may contain:

- controlled track sections,
- interlocking devices,
- active ETCS sessions,
- balise groups,
- GSM-R communication endpoints,
- diagnostic interfaces.

```text
+----------------------+
|      RBC Area        |
+----------------------+
        |
        +-- Interlocking
        +-- GSM-R
        +-- ETCS Sessions
        +-- MA Generator
        +-- Diagnostics
```

---

# ETCS Session Model

An ETCS session represents an active communication relation between onboard ETCS equipment and the RBC.

## Session Structure

| Field | Type | Description |
|---|---|---|
| `sessionID` | string | session identifier |
| `trainID` | string | train number |
| `rbcID` | string | assigned RBC |
| `status` | enum | session state |
| `position` | object | last reported position |
| `lastContact` | timestamp | timestamp of last valid communication |
| `etcsLevel` | enum | active ETCS level |
| `mode` | enum | train operating mode |

---

# Session States

| State | Meaning |
|---|---|
| `IDLE` | inactive session |
| `ESTABLISHING` | connection establishment |
| `ACTIVE` | active session |
| `SUSPENDED` | temporary communication loss |
| `TERMINATED` | terminated session |

---

# Movement Authority Model

Movement Authority (`MA`) defines the movement limit authorized by the RBC for a supervised train.

## MA Structure

```json
{
    "maID": "MA-WAR-000045",
    "trainID": "IC3812",
    "startLocation": "BG-1002",
    "endLocation": "BG-1044",
    "maxSpeedKmh": 160,
    "validUntil": "2026-05-21T15:22:00Z"
}
```

---

# RBC Commands

The supervisory system allows issuing operational commands to RBC.

| Command | Description |
|---|---|
| `REF` | refresh RBC data |
| `PGA` | operator authorization takeover |
| `EGA` | emergency authorization takeover |
| `CS` | RBC consistency check |
| `DIS` | restart RBC communication |
| `PAR` | switch RBC to passive mode |

---

# Command Confirmation

After issuing a command, an entry appears in the command list in `Sent` state.

## Command Confirmation

In case of successful execution:

1. RBC confirms the command.
2. Command state changes to `Confirmed`.
3. After the configured timeout the entry is removed from the list.

## Command Rejection

In case of failure:

1. RBC rejects the command.
2. Command state changes to `Error`.
3. The operator should verify RBC status and interlocking communication.

---

# Train Positioning

The system allows setting the approximate train position within the RBC area.

After selecting the positioning function:

- the station view becomes dimmed,
- signals allowing positioning are highlighted,
- the operator selects the signal corresponding to the approximate train position.

After selecting the object, the operator issues the `Set Position` command.

The command is transmitted to RBC as a two-stage operation.

---

# Positioning Conditions

Positioning may only be executed when:

- the operator has authorization for the area,
- an active ETCS session exists,
- communication with RBC is available,
- the selected signal belongs to the supervised area.

---

# Train Deregistration

The deregistration command removes the train from the RBC system.

The operation is used in case of:

- GSM-R communication loss,
- onboard ETCS failure,
- ETCS session loss,
- improper session termination.

After issuing the command:

- the ETCS session is terminated,
- RBC removes session data,
- further MA generation is blocked.

---

# Train Stop Command

The system allows issuing a train stop command.

After issuing the command:

- RBC sends a stop request,
- MA may be shortened,
- the train enters supervised braking mode.

The command may only be issued for an active ETCS session.

---

# Stop Cancellation

The `Cancel Stop` command cancels a previously issued stop request.

Cancellation is possible only when:

- RBC still supervises the train,
- an active ETCS session exists,
- no emergency state has been reached.

---

# Text Messages

The system allows sending text messages to ETCS trains.

Message content:

- may contain up to 255 characters,
- uses ETCS encoding,
- does not allow national diacritical characters.

## Allowed Characters

| Type | Range |
|---|---|
| Letters | A-Z |
| Digits | 0-9 |
| Special Characters | `. , : ; ? ! - /` |

After successful transmission, the message appears in the train message list.

---

# Message Status

| Status | Meaning |
|---|---|
| `SENT` | message sent |
| `DELIVERED` | message delivered |
| `FAILED` | transmission failure |
| `TIMEOUT` | confirmation timeout |

---

# Alarms and Diagnostic Messages

The system presents alarms originating from:

- RBC,
- interlocking systems,
- GSM-R infrastructure,
- DSAT systems,
- onboard ETCS equipment.

Alarms are displayed automatically after registration.

---

# Alarm Model

| Severity | Meaning |
|---|---|
| INFO | system information |
| WARNING | operational warning |
| CRITICAL | safety-related condition |
| EMERGENCY | immediate operator action required |

---

# Communication Supervision

The supervisory layer continuously monitors:

- GSM-R communication,
- RBC heartbeat,
- interlocking communication,
- ETCS session integrity,
- transmission errors.

## Communication Thresholds

| Event | Timeout |
|---|---|
| ETCS session timeout | 30 s |
| RBC heartbeat loss | 5 s |
| Interlocking communication loss | 3 s |

---

# Operator Authorization Model

Operational actions require obtaining control authority over the RBC area.

Operations requiring authorization:

- MA cancellation,
- train stop command,
- stop cancellation,
- train deregistration,
- train positioning,
- maintenance mode activation,
- RBC takeover.