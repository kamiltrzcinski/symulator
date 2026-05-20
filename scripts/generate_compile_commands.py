import os
import json
from pathlib import Path

repo_root = Path(__file__).resolve().parents[1]

# All include directories in the project
includes = [
    "server/include",
    "engine/include",
    "srk/common/include",
    "srk/ebilock/include",
    "srk/ml8/include",
    "libtrackview/include",
    "client/include",
    "editor/include",
    "autogens/proto",
    "3rdParty/vcpkg/installed/x64-windows/include",
    "3rdParty/asio/include",
]

# Convert includes to absolute paths
abs_includes = [str(repo_root / inc).replace("\\", "/") for inc in includes]

# Construct compiler flags
flags = ["-std=c++20", "-DASIO_STANDALONE"] + [f"-I{inc}" for inc in abs_includes]

# Scan for all C++ files
source_extensions = {".cpp", ".cc", ".cxx"}
commands = []

for root, dirs, files in os.walk(repo_root):
    # Skip build and 3rdparty directories
    if any(p in Path(root).parts for p in ["build", "vcpkg_installed", ".git", "3rdParty"]):
        continue
    for file in files:
        file_path = Path(root) / file
        if file_path.suffix in source_extensions:
            rel_file_path = file_path.relative_to(repo_root)
            command_entry = {
                "directory": str(repo_root).replace("\\", "/"),
                "command": f"clang++ {' '.join(flags)} -c {rel_file_path.as_posix()}",
                "file": rel_file_path.as_posix(),
            }
            commands.append(command_entry)

# Write compile_commands.json to root and build directories
destinations = [
    repo_root / "compile_commands.json",
    repo_root / "build" / "compile_commands.json",
    repo_root / "build" / "ninja-debug" / "compile_commands.json"
]

for dest in destinations:
    try:
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_text(json.dumps(commands, indent=2), encoding="utf-8")
        print(f"Generated {len(commands)} entries in {dest}")
    except Exception as e:
        print(f"Error writing to {dest}: {e}")
