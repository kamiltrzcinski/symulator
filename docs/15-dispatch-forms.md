# Dispatch Forms (S-Forms)

## Overview

Dispatch forms (Polish: *formularze zapowiedniowe*, prefix **S**) are the formal bilateral exchange protocol between neighbouring Line Control Sections (LCS). They implement the rule: **a train may not depart toward a neighbouring LCS until that LCS has confirmed the line is clear**.

In the simulator, this process is handled by the **Zapowiedniowiec** (Dispatch Form) module — a server-side component that manages the state machine of each bilateral exchange and produces/consumes `session.dispatch_telegrams` rows.

---

## Terminology

| Term | Meaning |
|---|---|
| **LCS** | Line Control Section (`SID`) — the basic organisational unit of track control |
| **Exchange** | One bilateral request/reply/confirm cycle between two LCS; grouped by `exchange_id` (UUID) |
| **Telegram** | One message in an exchange; stored as a row in `session.dispatch_telegrams` |
| **S-form** | Named form template (S2, S24, …); defines the content and allowed sequence of telegrams |
| **Droga wolna** | "Line clear" — confirmation from the receiving LCS that the line section is unoccupied and the train may be accepted |

---

## S-Form catalogue

| Form | Direction | Purpose |
|---|---|---|
| **S2** | A → B | Basic dispatch request: *"Can train N depart from A toward B?"* |
| **S24** | B → A | Reply granting line-clear for S2 |
| **S25** | A → B | Notification: train N has departed from A toward B |
| **S26** | B → A | Confirmation: train N has arrived at B (closes the exchange) |
| **S35** | A → B | Request to cancel/withdraw a previously sent S2 |
| **S51** | A → B | Level-crossing notification: train N will pass km markers listed |
| **S52** | B → A | Acknowledgement of S51 |
| **S55** | A → B | Extended dispatch request for trains carrying dangerous goods |
| **S56** | B → A | Reply to S55 |
| **S76** | A ↔ B | Free-form bilateral message (remarks, exceptions) |

---

## State machine for a standard exchange (S2 / S24 / S25 / S26)

```
[IDLE]
  │  Dispatcher at A initiates dispatch
  ▼
[S2_SENT]          A → B: "Czy droga dla manewrów z … na tor szlakowy … jest wolna?"
  │  B confirms line clear
  ▼
[S24_RECEIVED]     B → A: "Droga wolna" (grants departure)
  │  Train departs; A sends departure notification
  ▼
[S25_SENT]         A → B: "Pociąg N odjechał o godzinie …"
  │  Train arrives at B; B sends arrival confirmation
  ▼
[S26_RECEIVED]     B → A: "Pociąg N przyjechał o godzinie …"
  │
  ▼
[CLOSED]           Exchange complete; session.edr_entries.track_clear_time set at S24_RECEIVED step
```

Cancellation path:
```
[S2_SENT] → S35_SENT → [CANCELLED]
```

Dangerous-goods path replaces S2/S24 with S55/S56; otherwise identical.

---

## Duplicate-confirmation guard

When the server receives an S24 (or S56) and `session.edr_entries.track_clear_time` is already non-NULL for that `(session_id, train_number, station_sid)`, it:

1. Rejects the confirmation with status `REJECTED` in `session.dispatch_telegrams`.
2. Emits a warning event visible to the dispatcher: *"Rubryka 'droga wolna' dla pociągu nr X jest już wypełniona"* — exactly as seen in the Tymon reference screenshot.

This prevents double-entry errors without requiring the dispatcher to check manually.

---

## Level-crossing notifications (S51 / S52)

When a train passes through a section containing level crossings, the dispatching LCS must notify crossing wardens. The S51 form lists the km positions of crossings to be notified (e.g. 210.394, 212.705) and the estimated passage time.

These are stored in `session.dispatch_telegrams.km_markers` as a `TEXT[]` array. The client renders them in the **Przejazdy** column of the EDR view — one column per crossing.

The number and positions of crossings on each section are defined in the scenario topology (`scenarios/`). At session start the server pre-populates the S51 template for each section that has crossings.

---

## Server-side component: Zapowiedniowiec

`ZapowiedniowiecManager` is a server-side component (not a thread — it runs on DISPATCHER via callback, or as a service called from WORK_POOL). Its responsibilities:

- Validate that incoming S-form commands respect the allowed state-machine transitions.
- Reject out-of-order or duplicate telegrams with a structured error.
- Write `session.dispatch_telegrams` rows via DB_WRITER.
- Update `session.edr_entries.track_clear_time` on S24/S56 confirmation.
- Emit `DomainEvent::DispatchTelegramStateChanged` so DISPATCHER can push updates to affected clients.

`ZapowiedniowiecManager` does **not** own any engine state. It is a pure business-logic layer over the DB.

---

## Engine types

The following types in `engine/core/types.hpp` support this module:

| Type | Purpose |
|---|---|
| `TrainCategory` | `PASSENGER \| FREIGHT \| MAINTENANCE` — determines which S-form path applies |
| `DispatchFormType` | Enum of S2 / S24 / S25 / S26 / S35 / S51 / S52 / S55 / S56 / S76 |
| `TelegramDirection` | `SENT \| RECEIVED` |
| `TelegramStatus` | `PENDING \| CONFIRMED \| REJECTED \| SUPERSEDED` |
| `ExchangeStatus` | `IDLE \| S2_SENT \| S24_RECEIVED \| S25_SENT \| S26_RECEIVED \| CLOSED \| CANCELLED` |

---

## Open questions

- Q-SF-1: Should S51 km-marker lists be auto-generated from topology at dispatch time, or manually curated per-section in scenario data?
- Q-SF-2: Is S76 (free-form) stored as a telegram in the same table, or as a chat message in `session.chat_log`? Current decision: `dispatch_telegrams` with `form_type = 'S76'` to keep the audit trail together.
- Q-SF-3: Multi-LCS scenarios where a train crosses more than one LCS boundary before the previous S26 is received — does each hop open an independent exchange, or is there a chain? Current assumption: independent exchanges per LCS pair.
