#!/usr/bin/env bash
# generate_proto_headers.sh
#
# Re-generates FlatBuffers C++ headers from the proto/*.fbs schemas and writes
# them into autogens/proto/.
#
# WHEN TO RUN
#   Run this script whenever you edit any file in proto/.
#   Commit the updated headers together with the schema changes in the same PR.
#   CI validates that committed headers match the current schemas via
#   the `validate_proto_schemas` CMake target.
#
# USAGE (from repository root)
#   bash scripts/autogens/generate_proto_headers.sh
#
# OPTIONS
#   --flatc <path>   Path to the flatc binary.  If omitted, the script searches
#                    the vcpkg-installed binary, then PATH.
#   --dry-run        Print the flatc command without executing it.
#   -h, --help       Show this message and exit.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PROTO_DIR="${REPO_ROOT}/proto"
OUTPUT_DIR="${REPO_ROOT}/autogens/proto"

FLATC=""
DRY_RUN=0

# ── Argument parsing ──────────────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        --flatc)
            FLATC="$2"
            shift 2
            ;;
        --dry-run)
            DRY_RUN=1
            shift
            ;;
        -h|--help)
            sed -n '2,/^set -/p' "$0" | grep '^#' | sed 's/^# \?//'
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            exit 1
            ;;
    esac
done

# ── Locate flatc ──────────────────────────────────────────────────────────────
if [[ -z "$FLATC" ]]; then
    # Try vcpkg-installed binary first (common build/ layout)
    VCPKG_FLATC=""
    for candidate in \
        "${REPO_ROOT}/build/vcpkg_installed/x64-linux/tools/flatbuffers/flatc" \
        "${REPO_ROOT}/vcpkg_installed/x64-linux/tools/flatbuffers/flatc" \
        "${REPO_ROOT}/3rdParty/vcpkg/installed/x64-linux/tools/flatbuffers/flatc"; do
        if [[ -x "$candidate" ]]; then
            VCPKG_FLATC="$candidate"
            break
        fi
    done

    if [[ -n "$VCPKG_FLATC" ]]; then
        FLATC="$VCPKG_FLATC"
    elif command -v flatc &>/dev/null; then
        FLATC="$(command -v flatc)"
    else
        echo "ERROR: flatc not found." >&2
        echo "  Options:" >&2
        echo "    1. Build the project once (CMake fetches flatc via vcpkg)." >&2
        echo "    2. Install flatbuffers system-wide (e.g. apt install flatbuffers-compiler)." >&2
        echo "    3. Pass --flatc /path/to/flatc." >&2
        exit 1
    fi
fi

echo "Using flatc: ${FLATC}"
echo "  Schemas : ${PROTO_DIR}/*.fbs"
echo "  Output  : ${OUTPUT_DIR}"

# ── Collect schemas ───────────────────────────────────────────────────────────
SCHEMAS=()
for f in "${PROTO_DIR}"/*.fbs; do
    SCHEMAS+=("$f")
done

if [[ ${#SCHEMAS[@]} -eq 0 ]]; then
    echo "ERROR: No .fbs files found in ${PROTO_DIR}" >&2
    exit 1
fi

# ── Run flatc ─────────────────────────────────────────────────────────────────
CMD=(
    "$FLATC"
    --cpp
    --gen-object-api
    -I "${PROTO_DIR}"
    -o "${OUTPUT_DIR}"
    "${SCHEMAS[@]}"
)

if [[ $DRY_RUN -eq 1 ]]; then
    echo "DRY RUN — would execute:"
    printf '  %s\n' "${CMD[@]}"
    exit 0
fi

mkdir -p "${OUTPUT_DIR}"
"${CMD[@]}"

echo ""
echo "Done.  Generated files:"
ls -1 "${OUTPUT_DIR}"/*_generated.h 2>/dev/null || echo "  (none)"
echo ""
echo "Stage the changes:"
echo "  git add autogens/proto/"
