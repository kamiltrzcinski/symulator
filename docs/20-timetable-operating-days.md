# Timetable Operating Days Legend

Timetable entries store their operating days in the `operating_days` field. Values use ISO weekday numbers:

| Value | Weekday |
|---:|---|
| 1 | Monday |
| 2 | Tuesday |
| 3 | Wednesday |
| 4 | Thursday |
| 5 | Friday |
| 6 | Saturday |
| 7 | Sunday |

Common notation:

| Notation | Meaning |
|---|---|
| `1-7` / `{1,2,3,4,5,6,7}` | every day, Monday through Sunday |
| `1-5` / `{1,2,3,4,5}` | weekdays, Monday through Friday |
| `6,7` / `{6,7}` | weekend, Saturday and Sunday |

At startup, the server reads the current local date, converts it to an ISO weekday number, and copies only matching rows from `fleet.timetable_templates` into `session.edr_entries`. For example, a train with `operating_days = {1,2,3,4,5}` will not appear in a timetable started on Saturday.
