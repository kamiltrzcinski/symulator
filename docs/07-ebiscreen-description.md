# EbiScreen SRK System Specification

## Overview

EbiScreen: An SRK system operating a defined control district (area of control). It is intended for handling station devices and line devices at a control post or along a railway line section (LCS). Control of objects requires obtaining control rights for the area and explicit assignment of authorization areas by the operator for each control action.

Control mode is indicated by the color in which the station name is displayed:
- Yellow: control rights granted
- Grey: another supervisory system holds control rights
- Red: no supervisory system holds control rights

---

## Object Naming

Objects and their naming conventions:
- Signal: `syg_X` or `RS_X` (for LCS/group-controlled signals)
- Shunting signal: `tm_X`, `RS_TmX`, `syg_TmX`
- Switch: `zwr_X`
- Derailer: `wk_X`
- Line block: `bl_X`

---

## UID Structure

Each object is assigned a UID consisting of:
- `gID` – global unique identifier
- `pID` – operational identifier
- `sID` – station or LCS identifier
- `type` – object type (`syg`, `tm`, `zwr`, `wk`, `bl`)

---

## Operational Naming Rules (pID)

- LCS/group-controlled signals: `RS_<ID>` (e.g. `RS_A`)
- Individually controlled signals: `syg_<ID>` (e.g. `syg_A`)

Shunting signals:
- `RS_Tm<ID>` (e.g. `RS_Tm22`)
- `syg_Tm<ID>` (e.g. `syg_Tm22`)

Other objects:
- Switch: `zwr_<ID>`
- Derailer: `wk_<ID>`
- Line block: `bl_<ID>`

---

## Global Identifier (gID)

Format:
`TM-LCS_Or-RS_Tm22-0000001`

- type: `TM`
- area: `LCS_Or`
- pID: `RS_Tm22`
- UUID: `0000001`

---

## ID generation function

```

---

## Track section topology model

### Bidirectional track section

A track section (`OT`) connects two neighbours. Each end is called **sideA** or **sideB** — the labels carry no directional meaning. A train traverses the section in one of two directions, and the engine determines direction at runtime by knowing which side it entered from.

```
       sideA                              sideB
  [neighborID] ──[it/izID]── OT-xxx ──[it/izID]── [neighborID]
  signals[]                                         signals[]
```

Fields on each side:

| Field | Type | Notes |
|---|---|---|
| `neighborID` | string | gID of the adjacent track section, switch, or boundary node |
| `itID` | string | gID of the `It` axle counter — used when the neighbour is a **szlak boundary** (`BND-*`) |
| `izID` | string | gID of the `iz` axle counter — used when the neighbour is a **switch** (`ZWR-*`) |
| `signals` | string[] | gIDs of semaphores whose aspect controls entry into **this** section from **this** side |

Exactly one of `itID` / `izID` is present per side.

### Switch (rozjazd)

A switch has three named legs: `trunk` (pień), `straight` (wprost), `divergent` (odgałęzienie). Each leg uses the same structure as a track section side (`neighborID`, `izID`, `signals[]`). The switch also references its machine type via `typeID` from `data/device_types/`.

```json
{
    "gID":    "ZWR-TRJ-GDO-zwr1",
    "typeID": "DVT-GLB-ZWR-EEA4-0000002",
    "trunk":     { "neighborID": "OT-…-tor1a",  "izID": "IZ-…-zwr1-trunk",     "signals": [] },
    "straight":  { "neighborID": "OT-…-tor1b",  "izID": "IZ-…-zwr1-straight",  "signals": [] },
    "divergent": { "neighborID": "OT-…-peron1", "izID": "IZ-…-zwr1-divergent", "signals": [] },
    "lengthM": 28.5,
    "maxSpeedStraightKmh": 120,
    "maxSpeedDivergentKmh": 40
}
```

### Device type references

Lineside devices (signals, switch machines) are typed via `typeID` pointing to `data/device_types/`:

| `typeID` prefix | Device category | Example type file |
|---|---|---|
| `DVT-GLB-SEM-*` | Signal / semaphore | `data/device_types/semafor_ksztaltowy_pkp.json` |
| `DVT-GLB-ZWR-*` | Switch machine | `data/device_types/naped_eea4.json` |

Signal instances are defined in the scenario's `objects.json` (fixed to a location). Switch machine type is declared directly on the switch object in `topology.json`.cpp
string generateGID(string type, string area, string pID)
{
    string idNumber = padLeft(to_string(globalCounter), 7, '0');

    string gID =
        type + "-" +
        area + "-" +
        pID + "-" +
        idNumber;

    globalCounter++;

    return gID;
}
