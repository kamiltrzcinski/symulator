#!/usr/bin/env bash
# scripts/pre-commit-hook.sh
# Install into .git/hooks/pre-commit:
#   cp scripts/pre-commit-hook.sh .git/hooks/pre-commit
#   chmod +x .git/hooks/pre-commit

set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
RESET='\033[0m'

# ── 1. CHANGELOG.md must be staged ──────────────────────────────────────────
if ! git diff --cached --name-only | grep -q '^CHANGELOG\.md$'; then
    echo -e "${RED}[pre-commit] CHANGELOG.md is not staged.${RESET}"
    echo "  Update CHANGELOG.md with your changes, then:"
    echo "    git add CHANGELOG.md"
    echo "  If you are sure this commit needs no changelog entry (e.g. a work-in-progress"
    echo "  commit on a branch), bypass with:  git commit --no-verify"
    exit 1
fi

# ── 2. CHANGELOG.md must have a valid version entry ─────────────────────────
CHANGELOG_CONTENT=$(git show :CHANGELOG.md 2>/dev/null || cat CHANGELOG.md)
if ! echo "$CHANGELOG_CONTENT" | grep -qP '^## \[\d+\.\d+\.\d+\]'; then
    echo -e "${RED}[pre-commit] No valid version entry found in CHANGELOG.md.${RESET}"
    echo "  Add a line like:  ## [0.2.9] — $(date +%Y-%m-%d)"
    exit 1
fi

echo -e "${GREEN}[pre-commit] CHANGELOG.md OK.${RESET}"
exit 0
