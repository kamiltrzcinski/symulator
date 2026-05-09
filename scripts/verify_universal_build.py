#!/usr/bin/env python3
"""Validate cross-platform build automation by dry-running all host variants."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

HOST_SYSTEMS = ("Linux", "Windows", "Darwin")


def run_checked(command: list[str], *, cwd: Path) -> None:
    printable = " ".join(command)
    print(f"[run] {printable}")
    subprocess.run(command, cwd=cwd, check=True)


def main() -> int:
    repo_root = Path(__file__).resolve().parents[1]
    python_executable = sys.executable or "python3"

    for host_system in HOST_SYSTEMS:
        run_checked(
            [
                python_executable,
                "scripts/install_system_deps.py",
                "--host-system",
                host_system,
                "--print-only",
            ],
            cwd=repo_root,
        )

        run_checked(
            [
                python_executable,
                "scripts/configure_ninja.py",
                "--host-system",
                host_system,
                "--dry-run",
                "--build-type",
                "Debug",
                "--configure-only",
            ],
            cwd=repo_root,
        )

    print("\nUniversal host validation passed for Linux/Windows/macOS (dry-run mode).")
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
