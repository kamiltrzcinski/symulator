# ── SRK (System Sterowania Ruchem Kolejowym) libraries ──────────────────────
# This directory contains one library per supported interlocking system plus a
# common helper library shared by all of them.
#
# Build targets produced here:
#   srk_common   — topology graph traversal, shared interlocking helpers
#   srk_ebilock  — EbiLock X4 interlocking (IControlSystem implementation)
#   srk_ml8      — ESTW ML8 interlocking + SHL-12 block direction (IControlSystem)
#
# Each library links against the `engine` target for the IControlSystem and
# IStateView headers; none link against server or client code.

add_subdirectory(common)
add_subdirectory(ebilock)
add_subdirectory(ml8)
