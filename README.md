# Simulator

## POLSKI

Projekt symulatora sterowania ruchem kolejowym inspirowany workflow Ebilock.

Dokumentacja architektury i aktualnego stanu implementacji jest w
`docs/ARCHITECTURE.md`.

## Rozpoczęcie pracy

1. Zainstaluj zależności: `python3 scripts/install_system_deps.py`
   (git hooks aktywują się automatycznie przy pierwszym `cmake` — patrz
   `docs/ARCHITECTURE.md` §13).
2. Pobierz paczki z danymi:
   `python3 scripts/fetch_packages.py`
3. Przeczytaj `docs/ARCHITECTURE.md`.
4. Zbuduj projekt i uruchom testy — patrz `docs/ARCHITECTURE.md` §13.

> Paczki z danymi (`packages/`) są ignorowane przez git i pobierane automatycznie
> po każdym `git pull` (hook `post-merge`). Można je też pobrać ręcznie skryptem.

## Cel projektu

Stworzenie działającego, sieciowego symulatora SRK zawierającego:

- symulację stanów urządzeń sterowania ruchem i zajętości torów,
- realizację rozkładów jazdy,
- współdzielone sesje wielooperatorskie,
- bazę pod przyszłą automatyzację i AI.

## Licencja

Oprogramowanie własnościowe — patrz `LICENSE.md`. Komponenty firm trzecich
(m.in. Qt6) są licencjonowane odrębnie — patrz `THIRD_PARTY_NOTICES.md`.

---

## ENGLISH

A railway signaling simulation project inspired by Ebilock workflows.

Architecture and current implementation status are documented in
`docs/ARCHITECTURE.md`.

## Getting started

1. Install dependencies: `python3 scripts/install_system_deps.py`
   (git hooks activate automatically on the first `cmake` configure — see
   `docs/ARCHITECTURE.md` §13).
2. Fetch data packages:
   `python3 scripts/fetch_packages.py`
3. Read `docs/ARCHITECTURE.md`.
4. Build the project and run the tests — see `docs/ARCHITECTURE.md` §13.

> Data packages (`packages/`) are git-ignored and fetched automatically after
> every `git pull` via the `post-merge` hook. Run the script manually on first clone.

## Goal

Build a working networked signaling simulator with:

- signaling device states and track occupancy simulation,
- timetable execution,
- shared multi-operator sessions,
- a base for future automation and AI.

## License

Proprietary software — see `LICENSE.md`. Third-party components (including
Qt6) are licensed separately — see `THIRD_PARTY_NOTICES.md`.
