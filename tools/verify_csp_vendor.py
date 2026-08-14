#!/usr/bin/env python3
"""Verify libcsp and csp-rs485 vendored source pins."""

from __future__ import annotations

import hashlib
import subprocess
import sys
from pathlib import Path


EXPECTED_LIBCSP_COMMIT = "87006959696c78f70535ab382b0bcd4cb5a6558d"
EXPECTED_LIBCSP_URL = "https://github.com/libcsp/libcsp.git"
EXPECTED_CSP_RS485_SOURCE = Path(r"C:\PSC\csp-rs485")

MANIFEST = {
    "third_party/csp-rs485/include/csp_rs485_link.h": (
        "csp_rs485/include/csp_rs485_link.h",
        "31d742c857467afcfc7c1a78698b50266cb9a75fb804b4ab17b5cb6cfab2c88e",
    ),
    "third_party/csp-rs485/include/csp_rs485_port.h": (
        "csp_rs485/include/csp_rs485_port.h",
        "5ba347a46b3657e6b063c91b449fdf3ef2cac218b3495bc5758401f979cd4c58",
    ),
    "third_party/csp-rs485/include/csp_rs485_profile.h": (
        "csp_rs485/include/csp_rs485_profile.h",
        "6230fcfb0d2da73a03181ec31fdd456c125b9f60d0d97c708ba1127d02434e5a",
    ),
    "third_party/csp-rs485/src/csp_rs485_freertos.c": (
        "csp_rs485/src/csp_rs485_freertos.c",
        "3908669622668b477883f398e35409d6c8abe48666e816ad363f895f12ea6c8b",
    ),
    "third_party/csp-rs485/src/csp_rs485_internal.h": (
        "csp_rs485/src/csp_rs485_internal.h",
        "0b4ccc41672c3d1bf35cab95b3690a2653db593d1720328e0033e0daac98cd66",
    ),
    "third_party/csp-rs485/src/csp_rs485_kiss.c": (
        "csp_rs485/src/csp_rs485_kiss.c",
        "3c68200de98f7afbf2f08d90e975da71e367027e09785d90da32f8133a9702ed",
    ),
    "third_party/csp-rs485/src/csp_rs485_link.c": (
        "csp_rs485/src/csp_rs485_link.c",
        "d532dda4e430f47479ca9d638b16ef649f2dc93299a8149eecc4748a622544e6",
    ),
    "third_party/csp-rs485/src/csp_rs485_supervisor.c": (
        "csp_rs485/src/csp_rs485_supervisor.c",
        "0c332a16b444c80cdd33d1bac42eb0a6034618a16b1176527b9415580782129f",
    ),
}


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def fail(message: str) -> None:
    print(f"FAIL {message}", file=sys.stderr)
    raise SystemExit(1)


def run_git(args: list[str], cwd: Path) -> str:
    result = subprocess.run(
        ["git", *args],
        cwd=str(cwd),
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    if result.returncode != 0:
        fail(result.stderr.strip() or f"git {' '.join(args)} failed")
    return result.stdout.strip()


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for chunk in iter(lambda: file.read(65536), b""):
            digest.update(chunk)
    return digest.hexdigest()


def verify_gitmodules(root: Path) -> None:
    gitmodules = root / ".gitmodules"
    if not gitmodules.exists():
        fail(".gitmodules is missing")
    text = gitmodules.read_text(encoding="utf-8")
    required = [
        '[submodule "third_party/libcsp"]',
        "path = third_party/libcsp",
        f"url = {EXPECTED_LIBCSP_URL}",
    ]
    for item in required:
        if item not in text:
            fail(f".gitmodules missing {item!r}")


def verify_libcsp(root: Path) -> None:
    libcsp = root / "third_party" / "libcsp"
    if not libcsp.exists():
        fail("third_party/libcsp is missing")
    head = run_git(["rev-parse", "HEAD"], libcsp)
    if head != EXPECTED_LIBCSP_COMMIT:
        fail(f"libcsp pin mismatch: expected {EXPECTED_LIBCSP_COMMIT}, got {head}")


def verify_csp_rs485_files(root: Path) -> None:
    vendor_root = root / "third_party" / "csp-rs485"
    if not vendor_root.exists():
        fail("third_party/csp-rs485 is missing")

    expected_paths = {root / path for path in MANIFEST}
    actual_sources = {
        path
        for pattern in ("*.c", "*.h")
        for path in vendor_root.rglob(pattern)
    }
    extra = sorted(actual_sources - expected_paths)
    missing = sorted(expected_paths - actual_sources)
    if extra:
        fail("unexpected csp-rs485 source file: " + str(extra[0].relative_to(root)))
    if missing:
        fail("missing csp-rs485 source file: " + str(missing[0].relative_to(root)))

    for target_rel, (source_rel, expected_hash) in MANIFEST.items():
        target = root / target_rel
        actual_hash = sha256(target)
        if actual_hash != expected_hash:
            fail(f"{target_rel} hash mismatch: expected {expected_hash}, got {actual_hash}")
        source = EXPECTED_CSP_RS485_SOURCE / source_rel
        if source.exists() and target.read_bytes() != source.read_bytes():
            fail(f"{target_rel} differs from {source}")


def main() -> int:
    root = repo_root()
    verify_gitmodules(root)
    verify_libcsp(root)
    verify_csp_rs485_files(root)
    print(f"PASS libcsp={EXPECTED_LIBCSP_COMMIT} csp-rs485={len(MANIFEST)} files")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
