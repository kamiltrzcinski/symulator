#!/usr/bin/env python3
"""Install or print system dependencies required for local development."""

from __future__ import annotations

import argparse
import platform
import subprocess
import sys
from pathlib import Path

SCRIPT_BY_SYSTEM = {
    "Linux": ("bash", "scripts/deps/install_linux.sh"),
    "Darwin": ("bash", "scripts/deps/install_macos.sh"),
    "Windows": (
        "powershell",
        "-ExecutionPolicy",
        "Bypass",
        "-File",
        "scripts/deps/install_windows.ps1",
    ),
}

PRINT_ONLY_COMMANDS = {
    "Linux": [
        ["sudo", "apt-get", "update"],
        [
            "sudo",
            "apt-get",
            "install",
            "-y",
            "git",
            "cmake",
            "ninja-build",
            "gcc",
            "g++",
            "clang-format",
            "pkg-config",
            "zip",
            "curl",
            "unzip",
            "tar",
            "python3",
            "python3-venv",
            "autoconf",
            "autoconf-archive",
            "automake",
            "libtool",
            "libltdl-dev",
            "^libxcb.*-dev",
            "libx11-xcb-dev",
            "libglu1-mesa-dev",
            "libxrender-dev",
            "libxi-dev",
            "libxkbcommon-dev",
            "libxkbcommon-x11-dev",
            "libegl1-mesa-dev",
            "libice-dev",
            "libsm-dev",
            "libasio-dev",
            "libasound2-dev",
            "libpulse-dev",
            "libgstreamer1.0-dev",
            "libgstreamer-plugins-base1.0-dev",
            "libxrandr-dev",
            "nasm",
        ],
    ],
    "Darwin": [
        ["brew", "update"],
        [
            "brew",
            "install",
            "git",
            "cmake",
            "ninja",
            "llvm",
            "pkg-config",
            "python",
            "autoconf",
            "automake",
            "libtool",
            "asio",
        ],
    ],
    "Windows": [
        ["winget", "install", "--id", "Git.Git", "--exact", "--silent", "--accept-package-agreements", "--accept-source-agreements"],
        ["winget", "install", "--id", "Kitware.CMake", "--exact", "--silent", "--accept-package-agreements", "--accept-source-agreements"],
        ["winget", "install", "--id", "Ninja-build.Ninja", "--exact", "--silent", "--accept-package-agreements", "--accept-source-agreements"],
        ["winget", "install", "--id", "LLVM.LLVM", "--exact", "--silent", "--accept-package-agreements", "--accept-source-agreements"],
        ["winget", "install", "--id", "Python.Python.3.12", "--exact", "--silent", "--accept-package-agreements", "--accept-source-agreements"],
        [
            "winget",
            "install",
            "--id",
            "Microsoft.VisualStudio.2022.BuildTools",
            "--exact",
            "--silent",
            "--accept-package-agreements",
            "--accept-source-agreements",
            "--override",
            "--wait --quiet --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended",
        ],
    ],
}


def run_command(command: list[str], *, cwd: Path) -> None:
    printable = " ".join(command)
    print(f"[run] {printable}")
    subprocess.run(command, cwd=cwd, check=True)


def print_only_commands(system_name: str) -> None:
    for command in PRINT_ONLY_COMMANDS[system_name]:
        print(f"[print] {' '.join(command)}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Install system dependencies for the current platform.",
    )
    parser.add_argument(
        "--host-system",
        choices=["Linux", "Darwin", "Windows"],
        default=None,
        help="Override detected host OS (for print-only validation).",
    )
    parser.add_argument(
        "--print-only",
        action="store_true",
        help="Print commands instead of executing them.",
    )
    args = parser.parse_args()

    detected_system = platform.system()
    system_name = args.host_system or detected_system
    if system_name not in SCRIPT_BY_SYSTEM:
        raise RuntimeError(f"Unsupported host OS: {system_name}")

    if args.host_system and args.host_system != detected_system and not args.print_only:
        raise RuntimeError("--host-system different from current host is supported only with --print-only")

    repo_root = Path(__file__).resolve().parents[1]
    if args.print_only:
        print_only_commands(system_name)
        return 0

    command = list(SCRIPT_BY_SYSTEM[system_name])
    run_command(command, cwd=repo_root)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as error:
        print(f"\nCommand failed with exit code {error.returncode}.", file=sys.stderr)
        raise SystemExit(error.returncode)
    except Exception as error:  # pragma: no cover
        print(f"\nError: {error}", file=sys.stderr)
        raise SystemExit(1)
