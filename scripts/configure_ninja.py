#!/usr/bin/env python3
"""Configure Ninja builds with a shared third-party bootstrap directory.

This script keeps build directories generator-specific (to avoid CMake cache
mismatch) and optionally bootstraps vcpkg + dependencies, including Qt6.
"""

from __future__ import annotations

import argparse
import json
import os
import platform
import shlex
import shutil
import subprocess
import sys
import urllib.request
import zipfile
from pathlib import Path

THIRD_PARTY_DIR_NAME = "3rdParty"
# Standalone asio release — headers-only, no Boost dependency.
ASIO_VERSION_TAG = "asio-1-28-1"
ASIO_DOWNLOAD_URL = (
    f"https://github.com/chriskohlhoff/asio/archive/refs/tags/{ASIO_VERSION_TAG}.zip"
)
SUPPORTED_SYSTEMS = {"Windows", "Linux", "Darwin"}


def run_command(command: list[str], *, cwd: Path | None = None, env: dict[str, str] | None = None, dry_run: bool = False) -> None:
    printable = " ".join(shlex.quote(str(part)) for part in command)
    print(f"[run] {printable}")
    if dry_run:
        return
    subprocess.run(command, cwd=cwd, env=env, check=True)


def detect_third_party_root(repo_root: Path, system_name: str) -> Path:
    if system_name not in SUPPORTED_SYSTEMS:
        raise RuntimeError(f"Unsupported operating system: {system_name}")

    third_party_root = repo_root / THIRD_PARTY_DIR_NAME
    third_party_root.mkdir(parents=True, exist_ok=True)
    return third_party_root


def detect_triplet(system_name: str, machine_name: str, override: str | None, include_qt: bool) -> str:
    if override:
        return override

    machine = machine_name.lower()
    is_arm = "arm" in machine or "aarch64" in machine

    # Windows triplets are dynamic by default already — Qt (LGPLv3) has never
    # needed a static/dynamic split there. Linux/macOS stock triplets are
    # static by default, which is incompatible with distributing Qt in a
    # closed-source app without shipping relinkable object code (LGPLv3 §4).
    # When Qt is part of the build, switch those two platforms to vcpkg's
    # ready-made "-dynamic" community triplets instead.
    if system_name == "Windows":
        return "arm64-windows" if is_arm else "x64-windows"
    if system_name == "Darwin":
        base = "arm64-osx" if is_arm else "x64-osx"
        return f"{base}-dynamic" if include_qt else base
    if system_name == "Linux":
        base = "arm64-linux" if is_arm else "x64-linux"
        return f"{base}-dynamic" if include_qt else base

    raise RuntimeError(f"No default triplet mapping for OS: {system_name}")


def read_manifest_dependencies(vcpkg_manifest: Path, include_qt: bool, system_name: str) -> list[str]:
    data = json.loads(vcpkg_manifest.read_text(encoding="utf-8"))
    dependencies: list[str] = []

    is_windows = system_name == "Windows"
    is_linux = system_name == "Linux"

    for item in data.get("dependencies", []):
        name = ""
        features = []
        default_features = True
        platform_filter = ""

        if isinstance(item, str):
            name = item
        elif isinstance(item, dict) and isinstance(item.get("name"), str):
            name = item["name"]
            features = item.get("features", [])
            default_features = item.get("default-features", True)
            platform_filter = item.get("platform", "")

        if not name:
            continue

        # Evaluate platform filters
        if platform_filter:
            if platform_filter == "windows" and not is_windows:
                continue
            if platform_filter == "!windows" and is_windows:
                continue
            if platform_filter == "linux" and not is_linux:
                continue
            if platform_filter == "!linux" and is_linux:
                continue

        # Format package for classic mode
        if not default_features:
            all_features = ["core"] + features
            dependencies.append(f"{name}[{','.join(all_features)}]")
        elif features:
            dependencies.append(f"{name}[{','.join(features)}]")
        else:
            dependencies.append(name)

    if include_qt and "qtbase" not in [d.split("[")[0] for d in dependencies]:
        dependencies.append("qtbase")
    if not include_qt:
        dependencies = [dep for dep in dependencies if dep.split("[")[0] not in ("qtbase", "qtmultimedia")]

    # Preserve order while removing duplicates.
    deduplicated: list[str] = []
    seen: set[str] = set()
    for dep in dependencies:
        if dep not in seen:
            deduplicated.append(dep)
            seen.add(dep)

    return deduplicated


def ensure_vcpkg(third_party_root: Path, *, system_name: str, dry_run: bool) -> tuple[Path, Path]:
    vcpkg_root = third_party_root / "vcpkg"
    if not vcpkg_root.exists():
        run_command(
            [
                "git",
                "clone",
                "--depth",
                "1",
                "https://github.com/microsoft/vcpkg.git",
                str(vcpkg_root),
            ],
            dry_run=dry_run,
        )

    if system_name == "Windows":
        vcpkg_executable = vcpkg_root / "vcpkg.exe"
        bootstrap_script = vcpkg_root / "bootstrap-vcpkg.bat"
    else:
        vcpkg_executable = vcpkg_root / "vcpkg"
        bootstrap_script = vcpkg_root / "bootstrap-vcpkg.sh"

    if not vcpkg_executable.exists():
        if bootstrap_script.suffix == ".sh" and not dry_run:
            bootstrap_script.chmod(bootstrap_script.stat().st_mode | 0o111)

        run_command([str(bootstrap_script)], cwd=vcpkg_root, dry_run=dry_run)

    toolchain_file = vcpkg_root / "scripts" / "buildsystems" / "vcpkg.cmake"
    return vcpkg_executable, toolchain_file


def ensure_asio(third_party_root: Path, *, dry_run: bool) -> None:
    """Ensure standalone asio headers are available under 3rdParty/asio/include/.

    On Linux/macOS the native package manager (libasio-dev / brew install asio)
    installs asio into the system include path, which CMake finds automatically.
    This function is a cross-platform fallback: if asio.hpp is not present in
    the expected system paths it downloads the official headers-only release
    from GitHub and unpacks them into 3rdParty/asio/include/ so that CMake's
    ``find_path`` fallback can locate them without requiring a system package.
    """
    # System paths checked by CMake (see root CMakeLists.txt).
    system_candidates = [
        Path("/usr/include/asio.hpp"),
        Path("/usr/local/include/asio.hpp"),
    ]
    if any(p.exists() for p in system_candidates):
        return  # system package already provides asio

    target_include = third_party_root / "asio" / "include"
    asio_header = target_include / "asio.hpp"
    if asio_header.exists():
        print(f"[asio] already present at {target_include}")
        return

    zip_cache = third_party_root / f"{ASIO_VERSION_TAG}.zip"
    if dry_run:
        print(f"[run] download {ASIO_DOWNLOAD_URL} -> {zip_cache}")
        print(f"[run] extract asio headers to {target_include}")
        return

    print(f"[asio] downloading {ASIO_DOWNLOAD_URL} ...")
    target_include.mkdir(parents=True, exist_ok=True)
    urllib.request.urlretrieve(ASIO_DOWNLOAD_URL, zip_cache)  # noqa: S310

    with zipfile.ZipFile(zip_cache, "r") as zf:
        # The archive layout is: asio-asio-1-28-1/asio/include/asio.hpp (etc.)
        prefix = f"asio-{ASIO_VERSION_TAG}/asio/include/"
        for member in zf.namelist():
            if not member.startswith(prefix) or member == prefix:
                continue
            rel = member[len(prefix):]
            dest = target_include / rel
            if member.endswith("/"):
                dest.mkdir(parents=True, exist_ok=True)
            else:
                dest.parent.mkdir(parents=True, exist_ok=True)
                dest.write_bytes(zf.read(member))

    zip_cache.unlink(missing_ok=True)
    print(f"[asio] headers installed at {target_include}")


def install_third_party(
    *,
    vcpkg_executable: Path,
    dependencies: list[str],
    triplet: str,
    overlay_triplets_dir: Path,
    env: dict[str, str],
    dry_run: bool,
) -> None:
    if not dependencies:
        return

    run_command(
        [
            str(vcpkg_executable),
            "install",
            "--classic",
            *dependencies,
            f"--triplet={triplet}",
            f"--overlay-triplets={overlay_triplets_dir}",
            "--clean-after-build",
        ],
        env=env,
        dry_run=dry_run,
    )


def normalize_vcpkg_tools_layout(*, vcpkg_root: Path, triplet: str, dry_run: bool) -> None:
    """Mirror tools installed under usr/local/tools into the canonical tools path.

    Some autotools-based ports may install host tools under:
      installed/<triplet>/usr/local/tools/<name>/...
    while downstream ports expect:
      installed/<triplet>/tools/<name>/...
    """

    legacy_tools_root = vcpkg_root / "installed" / triplet / "usr" / "local" / "tools"
    if not legacy_tools_root.exists():
        return

    canonical_tools_root = vcpkg_root / "installed" / triplet / "tools"
    if dry_run:
        print(f"[run] mirror {legacy_tools_root} -> {canonical_tools_root}")
        return

    canonical_tools_root.mkdir(parents=True, exist_ok=True)
    for entry in legacy_tools_root.iterdir():
        destination = canonical_tools_root / entry.name
        if entry.is_dir():
            shutil.copytree(entry, destination, dirs_exist_ok=True)
        else:
            shutil.copy2(entry, destination)


def normalize_vcpkg_pkgconfig_layout(*, vcpkg_root: Path, triplet: str, dry_run: bool) -> None:
    """Mirror release pkg-config files into debug pkgconfig when missing.

    Some ports only ship release .pc metadata; meson debug configuration in
    downstream ports can fail if debug/lib/pkgconfig lacks matching entries.
    """

    release_pkgconfig_root = vcpkg_root / "installed" / triplet / "lib" / "pkgconfig"
    if not release_pkgconfig_root.exists():
        return

    debug_pkgconfig_root = vcpkg_root / "installed" / triplet / "debug" / "lib" / "pkgconfig"
    if dry_run:
        print(f"[run] mirror missing *.pc {release_pkgconfig_root} -> {debug_pkgconfig_root}")
        return

    debug_pkgconfig_root.mkdir(parents=True, exist_ok=True)
    for pc_file in release_pkgconfig_root.glob("*.pc"):
        destination = debug_pkgconfig_root / pc_file.name
        if not destination.exists():
            shutil.copy2(pc_file, destination)


def normalize_known_broken_pkgconfig_files(*, vcpkg_root: Path, triplet: str, dry_run: bool) -> None:
    """Fix malformed libcap/libpsx .pc metadata emitted by current toolchain combo."""

    release_pkgconfig_root = vcpkg_root / "installed" / triplet / "lib" / "pkgconfig"
    debug_pkgconfig_root = vcpkg_root / "installed" / triplet / "debug" / "lib" / "pkgconfig"
    known_broken = ("libcap.pc", "libpsx.pc")

    def rewrite_file(pc_file: Path, *, debug_layout: bool) -> None:
        if not pc_file.exists():
            return

        if dry_run:
            print(f"[run] normalize {pc_file}")
            return

        lines = pc_file.read_text(encoding="utf-8").splitlines()
        prefix_value = "${pcfiledir}/../../.." if debug_layout else "${pcfiledir}/../.."

        updated: list[str] = []
        saw_prefix = False
        for line in lines:
            if line.startswith("prefix="):
                updated.append(f"prefix={prefix_value}")
                saw_prefix = True
            elif line.startswith("exec_prefix="):
                updated.append("exec_prefix=${prefix}")
            elif line.startswith("libdir=/lib"):
                updated.append("libdir=${prefix}/lib")
            elif line.startswith("includedir=/include"):
                updated.append("includedir=${prefix}/include")
            else:
                updated.append(line)

        if not saw_prefix:
            updated.insert(0, f"prefix={prefix_value}")

        normalized_text = "\n".join(updated) + "\n"
        pc_file.write_text(normalized_text, encoding="utf-8")

    for filename in known_broken:
        rewrite_file(release_pkgconfig_root / filename, debug_layout=False)
        rewrite_file(debug_pkgconfig_root / filename, debug_layout=True)


def configure_ninja_build(
    *,
    repo_root: Path,
    build_dir: Path,
    build_type: str,
    generator: str,
    toolchain_file: Path | None,
    triplet: str,
    env: dict[str, str],
    dry_run: bool,
    configure_only: bool,
    headless: bool,
    build_tools: bool,
) -> None:
    command = [
        "cmake",
        "--fresh",
        "-S",
        str(repo_root),
        "-B",
        str(build_dir),
        "-G",
        generator,
        f"-DCMAKE_BUILD_TYPE={build_type}",
    ]

    if toolchain_file is not None:
        command.append(f"-DCMAKE_TOOLCHAIN_FILE={toolchain_file}")
        # We pre-install dependencies in classic mode.
        command.append("-DVCPKG_MANIFEST_MODE=OFF")
        # Without this, the vcpkg toolchain falls back to its own
        # host-detected default triplet (e.g. plain "x64-linux") instead of
        # the one we actually ran `vcpkg install --triplet=...` against —
        # harmless while only one triplet is ever installed, but silently
        # picks up a stale/incompatible triplet's packages (e.g. a static
        # Qt6 left over from a headless build) once more than one triplet
        # coexists under the same classic-mode installed/ tree.
        command.append(f"-DVCPKG_TARGET_TRIPLET={triplet}")

    if headless:
        command.extend([
            "-DBUILD_CLIENT=OFF",
            "-DBUILD_EDITOR=OFF",
        ])

    if build_tools:
        command.append("-DBUILD_TOOLS=ON")

    run_command(command, env=env, dry_run=dry_run)

    if not configure_only:
        run_command(["cmake", "--build", str(build_dir)], env=env, dry_run=dry_run)


def setup_msvc_environment(env: dict[str, str], dry_run: bool) -> None:
    if platform.system() != "Windows":
        return
    
    # Check if compiler is already in PATH
    if shutil.which("cl.exe"):
        return
        
    vswhere = os.path.expandvars(r"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe")
    if not os.path.exists(vswhere):
        return
        
    try:
        res = subprocess.run(
            [vswhere, "-latest", "-products", "*", "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", "-property", "installationPath"],
            capture_output=True, text=True, check=True
        )
        install_path = res.stdout.strip()
        if not install_path:
            return
            
        vcvars = os.path.join(install_path, "VC", "Auxiliary", "Build", "vcvarsall.bat")
        if not os.path.exists(vcvars):
            return
            
        if dry_run:
            print(f"[run] setup MSVC environment using {vcvars}")
            return
            
        cmd = f'"{vcvars}" x64 >nul 2>&1 && set'
        res = subprocess.run(cmd, shell=True, capture_output=True, text=True, check=True)
        
        for line in res.stdout.splitlines():
            if "=" in line:
                key, val = line.split("=", 1)
                key_upper = key.upper()
                if key_upper in ("PATH", "INCLUDE", "LIB", "LIBPATH"):
                    env[key_upper] = val
                    
        print(f"[msvc] environment initialized from {install_path}")
    except Exception as e:
        print(f"[msvc] failed to initialize environment: {e}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Configure a Ninja build and optionally bootstrap third-party dependencies.",
    )
    parser.add_argument("--build-type", default="Debug", help="CMAKE_BUILD_TYPE (default: Debug)")
    parser.add_argument("--build-dir", default=None, help="Build directory path (default: build/ninja-<type>)")
    parser.add_argument("--triplet", default=None, help="vcpkg triplet override (default: auto by host OS/arch)")
    parser.add_argument("--generator", default="Ninja", help="CMake generator (default: Ninja)")
    parser.add_argument("--headless", action="store_true", help="Configure server-only build (BUILD_CLIENT=OFF, BUILD_EDITOR=OFF)")
    parser.add_argument("--without-qt", action="store_true", help="Do not install qtbase via vcpkg")
    parser.add_argument("--build-tools", action="store_true", help="Also configure BUILD_TOOLS=ON (Qt developer tools)")
    parser.add_argument("--no-third-party-install", action="store_true", help="Skip vcpkg install step")
    parser.add_argument("--no-toolchain", action="store_true", help="Configure without vcpkg toolchain")
    parser.add_argument(
        "--host-system",
        choices=["Windows", "Linux", "Darwin"],
        default=None,
        help="Override detected host OS (for dry-run validation only)",
    )
    parser.add_argument("--configure-only", action="store_true", help="Run CMake configure only (skip build)")
    parser.add_argument("--dry-run", action="store_true", help="Print commands without executing them")
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parents[1]
    if args.build_dir:
        build_dir = Path(args.build_dir)
    else:
        suffix = "-headless" if args.headless else ""
        build_dir = repo_root / "build" / f"ninja-{args.build_type.lower()}{suffix}"

    actual_system = platform.system()
    system_name = args.host_system or actual_system
    if args.host_system and args.host_system != actual_system and not args.dry_run:
        raise RuntimeError("--host-system different from current host is supported only with --dry-run")

    include_qt = (not args.without_qt) and (not args.headless)

    third_party_root = detect_third_party_root(repo_root, system_name)
    triplet = detect_triplet(system_name, platform.machine(), args.triplet, include_qt)

    downloads_dir = third_party_root / "vcpkg-downloads"
    binary_cache_dir = third_party_root / "vcpkg-binary-cache"
    downloads_dir.mkdir(parents=True, exist_ok=True)
    binary_cache_dir.mkdir(parents=True, exist_ok=True)

    env = dict(os.environ)
    env["VCPKG_DOWNLOADS"] = str(downloads_dir)
    env["VCPKG_DEFAULT_BINARY_CACHE"] = str(binary_cache_dir)
    env["VCPKG_DISABLE_METRICS"] = "1"

    setup_msvc_environment(env, args.dry_run)

    toolchain_file: Path | None = None

    if not args.no_toolchain:
        vcpkg_executable, toolchain_file = ensure_vcpkg(
            third_party_root,
            system_name=system_name,
            dry_run=args.dry_run,
        )
        vcpkg_root = third_party_root / "vcpkg"
        env["VCPKG_ROOT"] = str(vcpkg_root)

        if not args.no_third_party_install:
            dependencies = read_manifest_dependencies(repo_root / "vcpkg.json", include_qt=include_qt, system_name=system_name)
            normalize_vcpkg_tools_layout(
                vcpkg_root=vcpkg_root,
                triplet=triplet,
                dry_run=args.dry_run,
            )
            normalize_vcpkg_pkgconfig_layout(
                vcpkg_root=vcpkg_root,
                triplet=triplet,
                dry_run=args.dry_run,
            )
            normalize_known_broken_pkgconfig_files(
                vcpkg_root=vcpkg_root,
                triplet=triplet,
                dry_run=args.dry_run,
            )
            install_third_party(
                vcpkg_executable=vcpkg_executable,
                dependencies=dependencies,
                triplet=triplet,
                overlay_triplets_dir=repo_root / "vcpkg-overlay-triplets",
                env=env,
                dry_run=args.dry_run,
            )
            normalize_vcpkg_tools_layout(
                vcpkg_root=vcpkg_root,
                triplet=triplet,
                dry_run=args.dry_run,
            )
            normalize_vcpkg_pkgconfig_layout(
                vcpkg_root=vcpkg_root,
                triplet=triplet,
                dry_run=args.dry_run,
            )
            normalize_known_broken_pkgconfig_files(
                vcpkg_root=vcpkg_root,
                triplet=triplet,
                dry_run=args.dry_run,
            )

    ensure_asio(third_party_root, dry_run=args.dry_run)

    configure_ninja_build(
        repo_root=repo_root,
        build_dir=build_dir,
        build_type=args.build_type,
        generator=args.generator,
        toolchain_file=toolchain_file,
        triplet=triplet,
        env=env,
        dry_run=args.dry_run,
        configure_only=args.configure_only,
        headless=args.headless,
        build_tools=args.build_tools,
    )

    print("\nDone.")
    print(f"Detected host system: {system_name}")
    print(f"Active third-party root: {third_party_root}")
    print(f"Configured build directory: {build_dir}")
    print(f"Detected triplet: {triplet}")
    print("Use 'cmake --build <build_dir>' and 'ctest --test-dir <build_dir> --output-on-failure'.")
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
