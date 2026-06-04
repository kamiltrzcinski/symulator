#!/usr/bin/env bash
# scripts/run_integration_tests.sh
#
# Spin up an ephemeral PostgreSQL container, run all CTest tests in
# integration groups (`group:integration-*`), then tear it down.
#
# Prerequisites:
#   - Docker with Compose v2 (docker compose)
#   - A compiled build in $BUILD_DIR (default: <repo>/build)
#
# Environment variables:
#   BUILD_DIR          Path to CMake build directory   (default: <repo>/build)
#   POSTGRES_USER      DB user                         (default: symulator)
#   POSTGRES_PASSWORD  DB password                     (default: symulator_test_pass)
#   POSTGRES_DB        DB name                         (default: symulator)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
COMPOSE_FILE="$REPO_ROOT/docker/docker-compose.test.yml"

DB_USER="${POSTGRES_USER:-symulator}"
DB_PASS="${POSTGRES_PASSWORD:-symulator_test_pass}"
DB_NAME="${POSTGRES_DB:-symulator}"
DB_PORT=5433

BUILD_DIR="${BUILD_DIR:-$REPO_ROOT/build}"

# ── Cleanup on exit ────────────────────────────────────────────────────────────
cleanup() {
    echo "[integration] Stopping test postgres..."
    docker compose -f "$COMPOSE_FILE" down --timeout 10 2>/dev/null || true
}
trap cleanup EXIT

# ── Start postgres ─────────────────────────────────────────────────────────────
echo "[integration] Starting ephemeral postgres on port $DB_PORT..."
POSTGRES_USER="$DB_USER" \
POSTGRES_PASSWORD="$DB_PASS" \
POSTGRES_DB="$DB_NAME" \
docker compose -f "$COMPOSE_FILE" up -d

# ── Wait for healthy ───────────────────────────────────────────────────────────
echo "[integration] Waiting for postgres to be ready..."
max_retries=30
for i in $(seq 1 $max_retries); do
    if docker compose -f "$COMPOSE_FILE" exec -T postgres \
           pg_isready -U "$DB_USER" -d "$DB_NAME" >/dev/null 2>&1; then
        echo "[integration] Postgres is ready (attempt $i/$max_retries)."
        break
    fi
    if [[ $i -eq $max_retries ]]; then
        echo "[integration] ERROR: postgres did not become ready after $max_retries attempts."
        exit 1
    fi
    sleep 1
done

# ── Run integration tests ──────────────────────────────────────────────────────
export SYMULATOR_TEST_DB="host=localhost port=$DB_PORT dbname=$DB_NAME user=$DB_USER password=$DB_PASS"

echo "[integration] Running integration test groups (BUILD_DIR=$BUILD_DIR)..."
ctest --test-dir "$BUILD_DIR" -L "group:integration-" --output-on-failure

echo "[integration] All integration tests passed."
