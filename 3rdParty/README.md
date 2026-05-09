This directory stores third-party artifacts managed by `scripts/configure_ninja.py`.

It is a shared root used on every platform (Linux, Windows, macOS) and contains:
- `vcpkg/` checkout
- `vcpkg-downloads/` cache
- `vcpkg-binary-cache/` compiled package cache

Do not commit downloaded binaries from this directory.
