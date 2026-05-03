# Changelog

All notable changes are documented here.

Entry format:
```
## [version] — YYYY-MM-DD
### Added / Changed / Fixed / Removed
- description
```

---

## [0.1.2] — 2026-05-03

### Added
- Vision and scope: station assignment model (5-6 stations, default 3 per player, tabbed client, permission-based takeover)
- System requirements: F-011 tabbed multi-panel client, F-012/F-013 station ownership and takeover, F-014 device behavior rules
- System requirements: N-008 SOLID at code and architecture level, N-009 replaceable transport layer (UDP candidate)
- System requirements: resolved open questions for AI module and permissions; added Naterki and UDP open questions
- Initial architecture: SOLID/modularity as core principles
- Initial architecture: tabbed panel design in Client UI component
- Initial architecture: AI Module defined as separate process from day one with explicit rationale
- Initial architecture: Transport layer options section (TCP/WS default vs UDP with custom framing, working decision)
- Starter backlog: P0 items for station list confirmation and station ownership protocol
- Starter backlog: P1 items for tabbed client layout and TCP vs UDP evaluation

## [0.1.1] — 2026-05-03

### Changed
- README: added one-time hook setup step for new contributors

## [0.1.0] — 2026-05-03

### Added
- Initial project planning documentation baseline
- Vision and scope with MVP definition and success criteria
- System requirements: functional (F-001–F-010), non-functional (N-001–N-007), data (D-001–D-004)
- Initial architecture: component split, communication contracts, deployment and rendering options
- Roadmap with 5 delivery stages (Stage 0–4) and definition of done per stage
- Starter backlog with P0/P1/P2 prioritization
- Dedicated server sizing baseline with first-principles load analysis and RPi4/RPi5 assessments
