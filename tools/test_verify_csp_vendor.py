"""Behavior tests for verify_csp_vendor.py using temporary Git repositories."""

from __future__ import annotations

import hashlib
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


APPROVED_LIBCSP = "87006959696c78f70535ab382b0bcd4cb5a6558d"
WRONG_LIBCSP = "1111111111111111111111111111111111111111"
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
SCRIPT = Path(__file__).with_name("verify_csp_vendor.py")


class VendorFixture:
    def __init__(self, base: Path) -> None:
        self.repo = base / "firmware repo"
        self.source = base / "source checkout"
        self.repo.mkdir()
        self.source.mkdir()
        subprocess.run(
            ["git", "init", "--quiet", str(self.repo)],
            check=True,
            capture_output=True,
            text=True,
        )
        self.set_gitlink(APPROVED_LIBCSP)

        rows = []
        for number, (target, source) in enumerate(FILE_MAP, start=1):
            content = f"fixture file {number}\n".encode()
            target_path = self.repo / "third_party" / "csp-rs485" / target
            source_path = self.source / source
            target_path.parent.mkdir(parents=True, exist_ok=True)
            source_path.parent.mkdir(parents=True, exist_ok=True)
            target_path.write_bytes(content)
            source_path.write_bytes(content)
            digest = hashlib.sha256(content).hexdigest()
            rows.append(f"| `{target}` | `{source}` | `{digest}` |")

        upstream = self.repo / "third_party" / "csp-rs485" / "UPSTREAM.md"
        upstream.write_text(
            "# Fixture provenance\n\n"
            "| Target path | Source path | SHA-256 |\n"
            "| --- | --- | --- |\n"
            + "\n".join(rows)
            + "\n",
            encoding="utf-8",
        )

    def set_gitlink(self, commit: str) -> None:
        subprocess.run(
            [
                "git",
                "-C",
                str(self.repo),
                "update-index",
                "--add",
                "--cacheinfo",
                f"160000,{commit},third_party/libcsp",
            ],
            check=True,
            capture_output=True,
            text=True,
        )

    def run(self) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            [
                sys.executable,
                str(SCRIPT),
                "--repo-root",
                str(self.repo),
                "--source-root",
                str(self.source),
            ],
            capture_output=True,
            text=True,
        )


class VerifyCspVendorTests(unittest.TestCase):
    def setUp(self) -> None:
        self.tempdir = tempfile.TemporaryDirectory(prefix="verify csp vendor ")
        self.addCleanup(self.tempdir.cleanup)
        self.fixture = VendorFixture(Path(self.tempdir.name))

    def assert_failure(self, expected: str) -> None:
        result = self.fixture.run()
        self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn(expected, result.stdout + result.stderr)

    def test_accepts_exact_vendor_intake(self) -> None:
        result = self.fixture.run()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(
            result.stdout.strip(),
            f"PASS libcsp={APPROVED_LIBCSP} csp-rs485=8 files",
        )
        self.assertEqual(result.stderr, "")

    def test_rejects_wrong_libcsp_gitlink(self) -> None:
        self.fixture.set_gitlink(WRONG_LIBCSP)
        self.assert_failure("libcsp gitlink")

    def test_rejects_changed_vendored_file(self) -> None:
        changed = self.fixture.repo / "third_party/csp-rs485/src/csp_rs485_link.c"
        changed.write_text("changed\n", encoding="utf-8")
        self.assert_failure("SHA-256 mismatch")

    def test_rejects_missing_vendored_file(self) -> None:
        missing = self.fixture.repo / "third_party/csp-rs485/include/csp_rs485_port.h"
        missing.unlink()
        self.assert_failure("missing vendored file")

    def test_rejects_extra_c_or_h_file(self) -> None:
        extra = self.fixture.repo / "third_party/csp-rs485/src/not_authorized.c"
        extra.write_text("extra\n", encoding="utf-8")
        self.assert_failure("extra C/header file")

    def test_rejects_difference_from_present_source_checkout(self) -> None:
        source = self.fixture.source / "csp_rs485/src/csp_rs485_supervisor.c"
        source.write_text("source changed\n", encoding="utf-8")
        self.assert_failure("differs from source checkout")


if __name__ == "__main__":
    unittest.main()
