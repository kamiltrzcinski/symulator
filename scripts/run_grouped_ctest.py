#!/usr/bin/env python3
"""Run CTest in logical groups and print PASS/FAILED summary per group."""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from pathlib import Path

GROUP_REGEX = {
    "unit-engine": r"^group:unit-engine-(foundation|concurrency|state|snapshot|fleet|simulation)$",
    "unit-server": r"^group:unit-server-protocol$|^group:unit-server-dispatch$",
    "unit-srk": r"^group:unit-srk-(ebilock|ml8|common)$",
    "unit-proto": r"^group:unit-proto$",
    "unit-validation": r"^group:unit-validation$",
    "integration-non-db": r"^group:integration-non-db$",
    "integration-db": r"^group:integration-db$",
    "unit-qt6": r"^group:unit-qt6$",
    "unit-libtrackview": r"^group:unit-libtrackview$",
    "tools": r"^tools$",
}

PROFILE_GROUPS = {
    "pre-db": [
        "unit-engine",
        "unit-server",
        "unit-srk",
        "unit-proto",
        "unit-validation",
        "integration-non-db",
    ],
    "db": ["integration-db"],
    "all": [
        "unit-engine",
        "unit-server",
        "unit-srk",
        "unit-proto",
        "unit-validation",
        "integration-non-db",
        "integration-db",
    ],
    "qt": ["unit-qt6", "unit-libtrackview", "tools"],
}


@dataclass
class GroupResult:
    name: str
    regex: str
    report_path: Path
    exit_code: int
    total: int
    failed: list[str]
    skipped: int


def parse_junit(report_path: Path) -> tuple[int, list[str], int]:
    if not report_path.exists():
        return 0, [], 0

    root = ET.parse(report_path).getroot()
    total = 0
    failed: list[str] = []
    skipped = 0

    for testcase in root.iter("testcase"):
        total += 1
        classname = testcase.attrib.get("classname", "")
        name = testcase.attrib.get("name", "")
        full_name = f"{classname}.{name}" if classname else name

        if testcase.find("failure") is not None or testcase.find("error") is not None:
            failed.append(full_name)
        elif testcase.find("skipped") is not None:
            skipped += 1

    return total, failed, skipped


def run_group(test_dir: Path, reports_dir: Path, group_name: str, regex: str) -> GroupResult:
    report_path = reports_dir / f"{group_name}.xml"

    cmd = [
        "ctest",
        "--test-dir",
        str(test_dir),
        "-L",
        regex,
        "--output-on-failure",
        "--output-junit",
        str(report_path),
    ]

    print(f"[group] Running {group_name} (label regex: {regex})")
    print(f"[group] Command: {' '.join(cmd)}")

    completed = subprocess.run(cmd, env=os.environ.copy(), check=False)
    total, failed, skipped = parse_junit(report_path)

    if completed.returncode == 0 and not failed:
        print(f"[PASS] {group_name} ({total} tests, {skipped} skipped)")
    else:
        print(f"[FAILED] {group_name} ({len(failed)} failed / {total} tests, {skipped} skipped)")
        for test_name in failed:
            print(f"  - {test_name}")

    return GroupResult(
        name=group_name,
        regex=regex,
        report_path=report_path,
        exit_code=completed.returncode,
        total=total,
        failed=failed,
        skipped=skipped,
    )


def write_github_summary(results: list[GroupResult]) -> None:
    summary_path = os.environ.get("GITHUB_STEP_SUMMARY")
    if not summary_path:
        return

    lines = [
        "## Grouped CTest Summary",
        "",
        "| Group | Result | Total | Failed | Skipped |",
        "|---|---:|---:|---:|---:|",
    ]

    for result in results:
        status = "PASS" if result.exit_code == 0 and not result.failed else "FAILED"
        lines.append(
            f"| {result.name} | {status} | {result.total} | {len(result.failed)} | {result.skipped} |"
        )

    failing = [r for r in results if r.exit_code != 0 or r.failed]
    if failing:
        lines.append("")
        lines.append("### Failed Tests")
        for result in failing:
            lines.append(f"- {result.name}")
            for test_name in result.failed:
                lines.append(f"  - {test_name}")

    with Path(summary_path).open("a", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--test-dir",
        required=True,
        help="CTest build directory, e.g. build/ninja-debug-headless",
    )
    parser.add_argument(
        "--reports-dir",
        default="test-reports",
        help="Directory for per-group JUnit reports",
    )
    parser.add_argument(
        "--profile",
        choices=sorted(PROFILE_GROUPS.keys()),
        default="all",
        help="Predefined group set to run",
    )
    parser.add_argument(
        "--group",
        action="append",
        default=[],
        help="Run only selected group (repeatable). Overrides --profile when provided.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    test_dir = Path(args.test_dir).resolve()
    reports_dir = Path(args.reports_dir).resolve()
    reports_dir.mkdir(parents=True, exist_ok=True)

    if args.group:
        groups = args.group
    else:
        groups = PROFILE_GROUPS[args.profile]

    unknown = [group for group in groups if group not in GROUP_REGEX]
    if unknown:
        print("Unknown groups: " + ", ".join(unknown), file=sys.stderr)
        return 2

    results: list[GroupResult] = []
    for group_name in groups:
        result = run_group(test_dir, reports_dir, group_name, GROUP_REGEX[group_name])
        results.append(result)

    print("\n[group] Final summary")
    failing_count = 0
    for result in results:
        status = "PASS" if result.exit_code == 0 and not result.failed else "FAILED"
        print(
            f"- {result.name}: {status} "
            f"(tests={result.total}, failed={len(result.failed)}, skipped={result.skipped})"
        )
        if status == "FAILED":
            failing_count += 1

    write_github_summary(results)

    if failing_count:
        print(f"\n[group] {failing_count} group(s) FAILED")
        return 1

    print("\n[group] All groups PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
