#!/usr/bin/env bash
set -euo pipefail

print_only=false
if [[ "${1:-}" == "--print-only" ]]; then
    print_only=true
fi

update_cmd=(sudo emerge --sync)
install_cmd=(
    sudo emerge -v --noreplace
    dev-vcs/git
    dev-build/cmake
    dev-build/ninja
    sys-devel/gcc
    llvm-core/clang
    virtual/pkgconfig
    app-arch/zip
    net-misc/curl
    app-arch/unzip
    app-arch/tar
    dev-lang/python
    dev-build/autoconf
    dev-build/autoconf-archive
    dev-build/automake
    dev-build/libtool
    x11-libs/libxcb
    x11-libs/libX11
    media-libs/glu
    x11-libs/libXrender
    x11-libs/libXi
    x11-libs/libxkbcommon
    media-libs/mesa
    x11-libs/libICE
    x11-libs/libSM
    dev-cpp/asio
    media-libs/alsa-lib
    media-libs/libpulse
    media-libs/gstreamer
    media-libs/gst-plugins-base
    x11-libs/libXrandr
    dev-lang/nasm
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