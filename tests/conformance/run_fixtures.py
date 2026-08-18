#!/usr/bin/env python3
"""Decode or reject frozen format fixtures listed in tests/fixtures/manifest.json."""

from __future__ import annotations

import hashlib
import json
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
import metadata

ROOT = metadata.ROOT
FIXTURE_DIR = ROOT / "tests" / "fixtures"
MANIFEST_PATH = FIXTURE_DIR / "manifest.json"
TIMEOUT_SECONDS = 10.0


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def decode(binary: Path, archive: Path, output: Path) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [str(binary), "decode", str(archive), str(output)],
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=TIMEOUT_SECONDS,
        check=False,
    )


def main() -> int:
    if not MANIFEST_PATH.is_file():
        raise SystemExit(f"missing fixture manifest: {MANIFEST_PATH}")

    manifest = json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))
    cases = manifest.get("cases")
    if not isinstance(cases, list) or not cases:
        raise SystemExit("fixture manifest has no cases")

    checks = 0
    with tempfile.TemporaryDirectory(prefix=".fixtures-", dir=metadata.TESTS_DIR) as tmp:
        tmpdir = Path(tmp)
        for case in cases:
            case_id = case["id"]
            algo = metadata.algorithm_by_name(case["algorithm"])
            archive = FIXTURE_DIR / case["archive"]
            if not archive.is_file():
                raise SystemExit(f"{case_id}: missing archive {archive}")
            if sha256(archive) != case["archive_sha256"]:
                raise SystemExit(f"{case_id}: archive hash mismatch")

            expect = case["expect"]
            decoded = tmpdir / f"{case_id}.out"
            proc = decode(algo.binary, archive, decoded)
            combined = proc.stdout + proc.stderr

            if expect == "decode":
                source = FIXTURE_DIR / case["input"]
                if not source.is_file():
                    raise SystemExit(f"{case_id}: missing input {source}")
                if sha256(source) != case["input_sha256"]:
                    raise SystemExit(f"{case_id}: input hash mismatch")
                if proc.returncode != 0:
                    raise SystemExit(
                        f"{case_id}: decode failed\nstdout:\n{proc.stdout}\nstderr:\n{proc.stderr}"
                    )
                if decoded.read_bytes() != source.read_bytes():
                    raise SystemExit(f"{case_id}: decoded bytes do not match input")
            elif expect == "reject-legacy":
                if proc.returncode == 0:
                    raise SystemExit(f"{case_id}: expected legacy rejection")
                if "legacy" not in combined.lower():
                    raise SystemExit(f"{case_id}: expected 'legacy' in error, got: {combined}")
                if "checksum" in combined.lower():
                    raise SystemExit(f"{case_id}: legacy rejection reported as checksum")
            elif expect == "reject-magic":
                if proc.returncode == 0:
                    raise SystemExit(f"{case_id}: expected bad-magic rejection")
                if "bad magic" not in combined.lower():
                    raise SystemExit(f"{case_id}: expected 'bad magic' in error, got: {combined}")
            else:
                raise SystemExit(f"{case_id}: unknown expect {expect!r}")

            checks += 1
            print(f"PASS fixture {case_id}")

    print(f"fixtures passed: {checks} case(s)")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.TimeoutExpired as exc:
        print(f"timed out: {' '.join(exc.cmd)}", file=sys.stderr)
        raise SystemExit(1)
    except RuntimeError as exc:
        print(str(exc), file=sys.stderr)
        raise SystemExit(1)
