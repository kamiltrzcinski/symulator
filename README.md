# Simulator

## POLSKI

Projekt symulatora sterowania ruchem kolejowym inspirowany workflow Ebilock.

Repozytorium znajduje się obecnie w fazie planowania. Pierwsza baza dokumentacji
jest dostępna w katalogu `docs/`.

## Rozpoczęcie pracy

1. Po sklonowaniu repozytorium aktywuj hook commitów:
   `git config core.hooksPath .githooks`
2. Przeczytaj indeks dokumentacji w `docs/README.md`.
3. Uzgodnij otwarte decyzje techniczne opisane w dokumencie architektury.
4. Podziel MVP na zadania i przypisz właścicieli.

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

1. After cloning, activate the commit hook once:
   `git config core.hooksPath .githooks`
2. Read the documentation index in `docs/README.md`.
3. Align on open technical decisions from the architecture document.
4. Break the MVP into issues and assign owners.

## Goal

Build a working networked signaling simulator with:

- signaling device states and track occupancy simulation,
- timetable execution,
- shared multi-operator sessions,
- a base for future automation and AI.
