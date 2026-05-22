# ESTW ML8 - System Description and Operator Commands

This document describes the dedicated command set for the `estw_ml8` control
system. ML8 is not treated as an EbiLock variant: it has its own command catalog,
its own transport command (`Ml8Command`, `cmd_type 0x21`), and dedicated handling
inside `libsrk_ml8`.

Source: the ML8 operator manual, chapter 11, "Operator commands in alphabetical
order".

Command names that contain characters unsuitable for C++ or FlatBuffers
identifiers are normalized: `STJ` and `STOJ` cover the stop-signal entry from the
manual, `OTEYYY` maps to `OTEyyy`, `OTPON` maps to `OTPOn`, `OT` maps to
`OTyyy[n]`, and hyphenated variants are stored as `OUZ_DR`, `Z_DR`, and so on.

## Simulator Model

- `engine::core::Ml8CommandCmd` represents an ML8 command inside the engine.
- `engine::core::Ml8CommandCode` contains the command mnemonics from the ML8
  operator manual.
- `proto::Ml8Command` is a separate FlatBuffers payload for client-server
  transport.
- `data/command_types/ml8_command_types.json` is the canonical JSON catalog for
  ML8 UI and scenario tooling.
- `srk::ml8::Ml8System` validates and executes ML8 commands on the ENGINE thread
  through the standard `IControlSystem` contract.

Commands backed by a physical simulator model mutate device state today:
signals, switches, derailers, track sections, axle counters, and line blocks.
The remaining commands are recorded as ML8 operator-command state so that client
and session logic can observe them without mixing them with EbiLock commands.

## Functional Groups

- Signals: `STJ`, `STOJ`, `STOP`, `SZ`, `NSZ`, `PZS`.
- Switches and derailers: `PZ`, `DPZ`, `PPZ`, `WPZ`, `KSR`, `STOP`, `OSTOP`.
- Tracks, sections, and axle counters: `ZEROLO`, `BLZC`, `POT`, `PZB`.
- Line blocks: `WBL`, `PZK`, `OWBL`, `BLZ`, `ZWBL`, `AZK`.
- Routes: `POC`, `MAN`, `ZDP`, `ZDM`, `ZCZ`, `ZW`, `PPN`, `WPN`.
- Workstation and station operation: `OP`, `OPO`, `DOP`, `DOPS`, `ZO`, `ZPO`,
  `LOFF`.
- Train number handling: `NPW`, `NPU`, `NPZ`.
- Diagnostics and test views: `AK`, `HMI`, `OTB`, `OTE`, `OTEYYY`, `OTP`,
  `OTPON`, `OT`, `OTZ`, `ZI`, `WZ`, `LKA`.
- Special and alarm events: `OGI`, `KRA`, `SPEC`.

## Command Catalog

| Code | Meaning |
| --- | --- |
| AK | Refresh fault information |
| AZK | Enable emergency block-direction change |
| BLZ | Restore the block to its normal state |
| BLZC | Restore vacancy detection control for a block section after a fault |
| DOP | Temporary takeover of operation |
| DOPS | Temporary takeover of station operation |
| DPZ | Temporary switch or derailer operation |
| HMI | HMI test |
| KRA | Theft alarm |
| KSR | Clear switch or derailer run-through indication |
| LKA | Clear the command line |
| LOFF | Log out from the active operator workstation |
| MAN | Shunting route |
| NPU | Remove train number |
| NPW | Enter train number |
| NPZ | Change train number |
| OGI | Fire alarm |
| OP | Take over operation |
| OPO | Hand over operation |
| OSTOP | Cancel stopping of a block, switch, derailer, crossing, or signal |
| OTB | System history / test image |
| OTE | Restore elements / test image |
| OTEYYY | Restore elements / test image, `OTEyyy` variant |
| OTP | Area preview / test image |
| OTPON | Area preview / test image, `OTPOn` variant |
| OT | Magnifier image / test image, `OTyyy[n]` variant |
| OTZ | Permission preview / test image |
| OUZ | Cancel an individual closure marker that prevents route setting |
| OUZ_DR | Cancel individual closure marker: track gang |
| OUZ_DZ | Cancel individual closure marker: draisine |
| OUZ_JN | Cancel individual closure marker: forbidden movement |
| OUZ_PJ | Cancel individual closure marker: overhead contact line |
| OUZ_X | Cancel individual closure marker: no reason specified |
| OUZ_ZN | Cancel individual closure marker: warning sign |
| OWBL | Cancel permission request |
| OZCZ | Cancel timed route release |
| P | Processing |
| POC | Train route |
| POT | Restore element after fault |
| PZK | Grant permission |
| PPN | Enable route setting |
| PPZ | Restore switch operation for an area |
| PZ | Operate switch or derailer |
| PZB | Confirm fault or error |
| PZS | Confirm signal activation after lamp replacement |
| SPEC | Special handling for mandatory level-1 registered commands |
| STJ | Extinguish permissive signal indication, normalized PDF code |
| STOJ | Extinguish permissive signal indication |
| STOP | Stop a block, switch, derailer, crossing, or signal |
| SZ | Display substitute signal |
| NSZ | Display substitute signal for departure onto the wrong track |
| WBL | Enable block / request permission |
| WPN | Disable route setting |
| WPZ | Disable switch operation for an area |
| WZ | Print individual closure markers |
| ZCZ | Timed route release |
| ZDM | Release shunting route |
| ZDP | Release train route |
| ZEROLO | Reset axle counter for a vacancy-detection section |
| ZI | Display information about individual closure markers |
| ZO | Offer operation |
| ZPO | Request operation |
| ZW | Temporary release of part of a route |
| ZWBL | Release block |
| Z | Add individual closure marker |
| Z_DR | Add individual closure marker: track gang |
| Z_DZ | Add individual closure marker: draisine |
| Z_JN | Add individual closure marker: forbidden movement |
| Z_PJ | Add individual closure marker: overhead contact line |
| Z_X | Add individual closure marker: no reason specified |
| Z_ZN | Add individual closure marker: warning sign |
