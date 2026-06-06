# Simulator

## POLSKI

Projekt symulatora sterowania ruchem kolejowym inspirowany workflow Ebilock.

Repozytorium znajduje się obecnie w fazie planowania. Pierwsza baza dokumentacji
jest dostępna w katalogu `docs/`.

## Rozpoczęcie pracy

1. Po sklonowaniu repozytorium aktywuj hooki:
   `git config core.hooksPath .githooks`
2. Pobierz paczki z danymi:
   `python3 scripts/fetch_packages.py`
3. Przeczytaj indeks dokumentacji w `docs/README.md`.
4. Uzgodnij otwarte decyzje techniczne opisane w dokumencie architektury.
5. Podziel MVP na zadania i przypisz właścicieli.

> Paczki z danymi (`packages/`) są ignorowane przez git i pobierane automatycznie
> po każdym `git pull` (hook `post-merge`). Można je też pobrać ręcznie skryptem.

## Cel projektu

Stworzenie działającego, sieciowego symulatora SRK zawierającego:

- symulację stanów urządzeń sterowania ruchem i zajętości torów,
- realizację rozkładów jazdy,
- współdzielone sesje wielooperatorskie,
- bazę pod przyszłą automatyzację i AI.

---

## ENGLISH

A railway signaling simulation project inspired by Ebilock workflows.

The repository is currently in planning mode. The first documentation baseline
is available in `docs/`.

## Getting started

1. After cloning, activate git hooks once:
   `git config core.hooksPath .githooks`
2. Fetch data packages:
   `python3 scripts/fetch_packages.py`
3. Read the documentation index in `docs/README.md`.
4. Align on open technical decisions from the architecture document.
5. Break the MVP into issues and assign owners.

> Data packages (`packages/`) are git-ignored and fetched automatically after
> every `git pull` via the `post-merge` hook. Run the script manually on first clone.

## Goal

Build a working networked signaling simulator with:

- signaling device states and track occupancy simulation,
- timetable execution,
- shared multi-operator sessions,
- a base for future automation and AI.
