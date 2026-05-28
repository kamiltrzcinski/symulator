# Legenda Dni Kursowania

Rozklad jazdy zapisuje dni kursowania w polu `operating_days`. Sa to numery dni tygodnia w standardzie ISO:

| Wartosc | Dzien tygodnia |
|---:|---|
| 1 | poniedzialek |
| 2 | wtorek |
| 3 | sroda |
| 4 | czwartek |
| 5 | piatek |
| 6 | sobota |
| 7 | niedziela |

Najczestsze zapisy:

| Zapis | Znaczenie |
|---|---|
| `1-7` / `{1,2,3,4,5,6,7}` | codziennie, od poniedzialku do niedzieli |
| `1-5` / `{1,2,3,4,5}` | dni robocze, od poniedzialku do piatku |
| `6,7` / `{6,7}` | weekend, sobota i niedziela |

Przy starcie serwer odczytuje aktualna lokalna date, wylicza numer dnia tygodnia i przenosi do `session.edr_entries` tylko te wpisy z `fleet.timetable_templates`, ktore zawieraja ten numer w `operating_days`. Przyklad: sklad z `operating_days = {1,2,3,4,5}` nie pojawi sie w rozkladzie uruchomionym w sobote.
