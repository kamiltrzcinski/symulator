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