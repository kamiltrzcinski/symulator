# UID Generator

`uid-generator` is a Qt6 Widgets application for creating structured,
collision-free UIDs.

## Installation and Launch

Build the project with `-DBUILD_TOOLS=ON`. Qt6 Widgets must be available as
dynamic libraries at runtime.

```sh
uid-generator [--data-dir <path>] [--help]
```

With no `--data-dir`, the application reads `packages/vehicle-types/`,
`packages/vehicles/` and `packages/trains/` below the current working
directory. With `--data-dir`, it recursively recognizes VehicleType, Vehicle
and Train JSON documents below the supplied directory. Invalid, unrelated or
unreadable JSON files are skipped.

The active source can also be changed with **File > Open Directory...**.

## Generate a UID

1. Select a Domain.
2. Select a Kind. The list only contains Kinds valid for the selected Domain.
3. Enter `SCOPE` in the range `0` through `65535`.
4. Enter the first `INSTANCE` in the range `1` through `65535`.
5. Select **Generate**.

The result view shows decimal and `0x`-prefixed hexadecimal forms. **Copy
decimal UID** places the decimal value on the system clipboard.

For rolling-stock UIDs, `SCOPE=0` means global and produces a non-blocking
advisory. `INSTANCE=0` is invalid and disables generation.

## Collision Guard

Before returning a UID, `UidGeneratorService` checks the loaded registry. If
the requested instance is occupied, the service automatically advances to the
next free positive instance, wrapping within the 16-bit range if necessary.
The UI never receives an occupied UID.

If no instance is available in the selected `(Domain, Kind, SCOPE)` bucket,
generation is disabled until the user changes the SCOPE or Kind.

## Registry and Legend

The **UID Registry** tab lists the UID, decoded fields and source file for
loaded records and references. The **UID Legend** tab is available immediately
at startup, even when no data source could be loaded. It shows the bit layout
and every Domain/Kind/SCOPE combination from the compile-time legend table.
