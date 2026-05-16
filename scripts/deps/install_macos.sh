#!/usr/bin/env bash
set -euo pipefail

print_only=false
if [[ "${1:-}" == "--print-only" ]]; then
    print_only=true
fi

if ! command -v brew >/dev/null 2>&1; then
    echo "Homebrew is required. Install it from https://brew.sh" >&2
    exit 1
fi

update_cmd=(brew update)
install_cmd=(
    brew install
    git
    cmake
    ninja
    llvm
    pkg-config
    python
    autoconf
    automake
    libtool
    asio
)

if [[ "${print_only}" == true ]]; then
    printf '[print]'
    printf ' %q' "${update_cmd[@]}"
    printf '\n'
    printf '[print]'
    printf ' %q' "${install_cmd[@]}"
    printf '\n'
    exit 0
fi

"${update_cmd[@]}"
"${install_cmd[@]}"

echo "macOS dependencies installed."
