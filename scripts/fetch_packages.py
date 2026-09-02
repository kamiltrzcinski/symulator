#!/usr/bin/env python3
"""
Pobiera najnowsze paczki danych z GitHub Releases (kamiltrzcinski/symulator-data).
Usuwa stare wersje paczek, rozpakowuje nowe do packages/.
"""

import os
import sys
import json
import tarfile
import shutil
import urllib.request
import urllib.error
from pathlib import Path

REPO = "kamiltrzcinski/symulator-data"
PACKAGES_DIR = Path(__file__).parent.parent / "packages"
API_URL = f"https://api.github.com/repos/{REPO}/releases/latest"

KNOWN_PACKAGES = ["trains", "vehicles", "vehicle-types", "schedules", "carriers", "timetable-points", "timetable-connections"]

GITHUB_TOKEN = os.environ.get("GITHUB_TOKEN", "")


def _headers():
    h = {"User-Agent": "symulator-fetch"}
    if GITHUB_TOKEN:
        h["Authorization"] = f"Bearer {GITHUB_TOKEN}"
    return h


def fetch_latest_release():
    req = urllib.request.Request(API_URL, headers=_headers())
    try:
        with urllib.request.urlopen(req) as resp:
            return json.loads(resp.read())
    except urllib.error.HTTPError as e:
        print(f"Błąd pobierania release: {e}", file=sys.stderr)
        sys.exit(1)


def current_versions():
    versions = {}
    for pkg in KNOWN_PACKAGES:
        version_file = PACKAGES_DIR / pkg / ".version"
        if version_file.exists():
            versions[pkg] = version_file.read_text().strip()
    return versions


def download_and_extract(asset_name, download_url, pkg_name):
    tmp_path = PACKAGES_DIR / f"_tmp_{asset_name}"
    pkg_dir = PACKAGES_DIR / pkg_name

    print(f"  Pobieranie {asset_name}...")
    req = urllib.request.Request(download_url, headers=_headers())
    with urllib.request.urlopen(req) as resp, open(tmp_path, "wb") as f:
        shutil.copyfileobj(resp, f)

    if pkg_dir.exists():
        shutil.rmtree(pkg_dir)
    pkg_dir.mkdir(parents=True)

    with tarfile.open(tmp_path, "r:gz") as tar:
        # strip leading data/<pkg>/ from paths
        for member in tar.getmembers():
            parts = Path(member.name).parts
            # strip first two path components (data/<pkg-name>/)
            if len(parts) > 2:
                member.name = str(Path(*parts[2:]))
                tar.extract(member, pkg_dir)

    tmp_path.unlink()

    # zapisz wersję
    version = asset_name.split(".tar.gz")[0]  # e.g. trains.0.1.1.20260606.3
    version = ".".join(version.split(".")[1:])  # 0.1.1.20260606.3
    (pkg_dir / ".version").write_text(version)
    print(f"  OK: {pkg_name} -> {version}")


def main():
    PACKAGES_DIR.mkdir(exist_ok=True)

    print("Sprawdzanie najnowszego release...")
    release = fetch_latest_release()
    tag = release["tag_name"]
    assets = {a["name"]: a["browser_download_url"] for a in release["assets"]}

    current = current_versions()

    updated = False
    for asset_name, url in assets.items():
        if not asset_name.endswith(".tar.gz"):
            continue

        # wyciągnij nazwę paczki (pierwsza część przed pierwszą kropką)
        pkg_name = asset_name.split(".")[0]
        if pkg_name not in KNOWN_PACKAGES:
            continue

        version = ".".join(asset_name.replace(".tar.gz", "").split(".")[1:])

        if current.get(pkg_name) == version:
            print(f"  {pkg_name}: aktualny ({version}), pomijam")
            continue

        download_and_extract(asset_name, url, pkg_name)
        updated = True

    # usuń paczki których nie ma w release (np. nazwa się zmieniła)
    for pkg in KNOWN_PACKAGES:
        pkg_dir = PACKAGES_DIR / pkg
        if pkg_dir.exists() and not any(a.startswith(pkg + ".") for a in assets):
            print(f"  Usuwam starą paczkę: {pkg}")
            shutil.rmtree(pkg_dir)

    if not updated:
        print("Wszystkie paczki są aktualne.")
    else:
        print(f"Paczki zaktualizowane do release {tag}.")


if __name__ == "__main__":
    main()
