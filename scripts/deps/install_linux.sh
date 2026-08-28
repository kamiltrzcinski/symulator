#!/usr/bin/env bash
set -euo pipefail

print_only=false
if [[ "${1:-}" == "--print-only" ]]; then
    print_only=true
fi

update_cmd=(sudo apt-get update)
install_cmd=(
    sudo apt-get install -y
    git
    cmake
    ninja-build
    gcc
    g++
    clang-format
    pkg-config
    zip
    curl
    unzip
    tar
    python3
    python3-venv
    autoconf
    autoconf-archive
    automake
    libtool
    libltdl-dev
    '^libxcb.*-dev'
    libx11-xcb-dev
    libglu1-mesa-dev
    libxrender-dev
    libxi-dev
    libxkbcommon-dev
    libxkbcommon-x11-dev
    libegl1-mesa-dev
    libice-dev
    libsm-dev
    libasio-dev
    libasound2-dev
    libpulse-dev
    libgstreamer1.0-dev
    libgstreamer-plugins-base1.0-dev
    nasm
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

echo "Linux dependencies installed."
