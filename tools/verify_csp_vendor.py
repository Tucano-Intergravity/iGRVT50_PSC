#!/usr/bin/env python3
"""Verify pinned CSP vendor dependencies."""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import re
import subprocess
import sys


APPROVED_LIBCSP = "87006959696c78f70535ab382b0bcd4cb5a6558d"
DEFAULT_SOURCE_ROOT = Path(r"C:\PSC\csp-rs485")
FILE_MAP = (
    ("include/csp_rs485_link.h", "csp_rs485/include/csp_rs485_link.h"),
    ("include/csp_rs485_port.h", "csp_rs485/include/csp_rs485_port.h"),
    ("include/csp_rs485_profile.h", "csp_rs485/include/csp_rs485_profile.h"),
    ("src/csp_rs485_internal.h", "csp_rs485/src/csp_rs485_internal.h"),
    ("src/csp_rs485_freertos.c", "csp_rs485/src/csp_rs485_freertos.c"),
    ("src/csp_rs485_kiss.c", "csp_rs485/src/csp_rs485_kiss.c"),
    ("src/csp_rs485_link.c", "csp_rs485/src/csp_rs485_link.c"),
    ("src/csp_rs485_supervisor.c", "csp_rs485/src/csp_rs485_supervisor.c"),
)
MANIFEST_ROW = re.compile(
    r"^\|\s*`([^`]+)`\s*\|\s*`([^`]+)`\s*\|\s*`([0-9a-fA-F]{64})`\s*\|\s*$"
)


class VerificationError(RuntimeError):
    """A vendor intake invariant was not satisfied."""


def run_git(repo_root: Path, *arguments: str) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(
        ["git", "-C", str(repo_root), *arguments],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        detail = result.stderr.strip() or result.stdout.strip()
        raise VerificationError(f"git command failed: {detail}")
    return result


def verify_libcsp_gitlink(repo_root: Path) -> None:
    result = run_git(
        repo_root,
        "ls-files",
        "--stage",
        "--",
        "third_party/libcsp",
    )
    match = re.fullmatch(
        r"160000 ([0-9a-f]{40}) 0\tthird_party/libcsp\r?\n?",
        result.stdout,
    )
    if match is None:
        raise VerificationError("libcsp gitlink is missing or is not a submodule")
    actual = match.group(1)
    if actual != APPROVED_LIBCSP:
        raise VerificationError(
            f"libcsp gitlink is {actual}; expected {APPROVED_LIBCSP}"
        )


def read_manifest(upstream_path: Path) -> dict[str, tuple[str, str]]:
    if not upstream_path.is_file():
        raise VerificationError(f"missing provenance file: {upstream_path}")

    manifest: dict[str, tuple[str, str]] = {}
    for line in upstream_path.read_text(encoding="utf-8").splitlines():
        match = MANIFEST_ROW.fullmatch(line)
        if match is None:
            continue
        target, source, digest = match.groups()
        if target in manifest:
            raise VerificationError(f"duplicate manifest path: {target}")
        manifest[target] = (source, digest.lower())

    expected = {target: source for target, source in FILE_MAP}
    actual = {target: source for target, (source, _digest) in manifest.items()}
    if actual != expected:
        missing = sorted(set(expected) - set(actual))
        extra = sorted(set(actual) - set(expected))
        wrong_sources = sorted(
            target
            for target in set(expected) & set(actual)
            if expected[target] != actual[target]
        )
        details = []
        if missing:
            details.append(f"missing={','.join(missing)}")
        if extra:
            details.append(f"extra={','.join(extra)}")
        if wrong_sources:
            details.append(f"wrong-source={','.join(wrong_sources)}")
        raise VerificationError("invalid eight-file manifest: " + "; ".join(details))
    return manifest


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(128 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def verify_vendor_files(
    vendor_root: Path, manifest: dict[str, tuple[str, str]]
) -> None:
    for target, _source in FILE_MAP:
        path = vendor_root / target
        if not path.is_file():
            raise VerificationError(f"missing vendored file: {target}")
        actual = sha256(path)
        expected = manifest[target][1]
        if actual != expected:
            raise VerificationError(
                f"SHA-256 mismatch for {target}: {actual}; expected {expected}"
            )

    allowed = {target for target, _source in FILE_MAP}
    present = {
        path.relative_to(vendor_root).as_posix()
        for path in vendor_root.rglob("*")
        if path.is_file() and path.suffix.lower() in {".c", ".h"}
    }
    extra = sorted(present - allowed)
    if extra:
        raise VerificationError("extra C/header file: " + ", ".join(extra))


def verify_source_diff(
    source_root: Path,
    vendor_root: Path,
    manifest: dict[str, tuple[str, str]],
) -> None:
    if not source_root.exists():
        return
    if not source_root.is_dir():
        raise VerificationError(f"source checkout is not a directory: {source_root}")

    for target, (source, _digest) in manifest.items():
        source_path = source_root / source
        if not source_path.is_file():
            raise VerificationError(f"missing source-checkout file: {source}")
        target_path = vendor_root / target
        result = subprocess.run(
            [
                "git",
                "diff",
                "--no-index",
                "--no-ext-diff",
                "--exit-code",
                "--",
                str(source_path),
                str(target_path),
            ],
            capture_output=True,
            text=True,
        )
        if result.returncode == 1:
            raise VerificationError(
                f"vendored file differs from source checkout: {target}"
            )
        if result.returncode != 0:
            detail = result.stderr.strip() or result.stdout.strip()
            raise VerificationError(f"git diff --no-index failed for {target}: {detail}")


def verify(repo_root: Path, source_root: Path) -> None:
    vendor_root = repo_root / "third_party" / "csp-rs485"
    verify_libcsp_gitlink(repo_root)
    manifest = read_manifest(vendor_root / "UPSTREAM.md")
    verify_vendor_files(vendor_root, manifest)
    verify_source_diff(source_root, vendor_root, manifest)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repo-root",
        type=Path,
        default=Path(__file__).resolve().parents[1],
        help="repository to verify (default: parent of this tools directory)",
    )
    parser.add_argument(
        "--source-root",
        type=Path,
        default=DEFAULT_SOURCE_ROOT,
        help="optional csp-rs485 source checkout used for no-index diffs",
    )
    args = parser.parse_args()
    try:
        verify(args.repo_root.resolve(), args.source_root.resolve())
    except VerificationError as error:
        print(f"FAIL {error}", file=sys.stderr)
        return 1
    print(f"PASS libcsp={APPROVED_LIBCSP} csp-rs485=8 files")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
