# Vehicle Browser

`vehicle-browser` is a Qt6 Widgets application for browsing VehicleTypes and
Vehicles, creating Vehicle JSON files, and composing Train consists.

## Installation and Launch

Build with `-DBUILD_TOOLS=ON`. Qt6 Widgets must be available as dynamic
libraries at runtime.

```sh
vehicle-browser [--data-dir <path>] [--help]
```

Without `--data-dir`, data is loaded from `packages/` below the current
working directory. With `--data-dir`, all recognized JSON documents below the
provided directory are loaded recursively. Use **File > Open Directory...**
to switch sources without restarting.

Malformed, unrelated and unreadable JSON documents are skipped.

## Browse Data

The Vehicle Types panel supports case-insensitive filtering across all visible
columns and sorting by clicking a column heading. Selecting a VehicleType
filters the Vehicles panel to matching `type_uid` values. Clearing the
selection shows all Vehicles.

## Create a Vehicle

1. Select a VehicleType.
2. Choose **New Vehicle...**.
3. Enter the side number (`pID`).
4. Optionally enter carrier UID, inventory number and notes.
5. Select **Save** and choose an output directory.

The dialog copies the selected VehicleType UID and proposes a collision-free
`VEHICLE` UID. Availability is checked again before writing. The output is
stored as `<output>/<safe-side-number>/vehicle.json`.

Example:

```json
{
  "uid": 1108101562369,
  "type_uid": 1103806595073,
  "pID": "EU07-001",
  "displayName": "EU07-001",
  "carrierId": 1116691496961,
  "inventoryNumber": "91 51 5 140 001-0",
  "notes": "Optional text"
}
```

Required fields are `uid`, `type_uid`, `pID` and `displayName`. The remaining
fields shown above are optional.

## Compose and Save a Train

Select a Vehicle and choose **Add to Train**. The Train consist list supports
internal drag-and-drop reordering and removal. Choose **Save Train...**, enter
the train number, display name and category, then select the output file.

A collision-free `TRAIN_CONSIST` UID is generated immediately before writing.

Example:

```json
{
  "uid": 1112396529665,
  "pID": "T1",
  "displayName": "Freight train T1",
  "trainCategory": "FREIGHT",
  "vehicle_uids": [
    1108101562369
  ]
}
```

Required fields are `uid`, `pID`, `displayName`, `trainCategory` and
`vehicle_uids`. `carrierId` may be added by compatible data producers.

## UID Legend

Choose **Help > UID Legend** at any time. The legend is independent of the
active data source and displays the complete UID bit layout and reference
table.
