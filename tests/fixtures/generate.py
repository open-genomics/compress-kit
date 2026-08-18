#!/usr/bin/env python3
"""Generate frozen v1-rejection and v2-decode fixtures plus manifest hashes."""

from __future__ import annotations

import hashlib
import json
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
import metadata

ROOT = metadata.ROOT
FIXTURE_DIR = ROOT / "tests" / "fixtures"
TIMEOUT_SECONDS = 10.0

INPUTS = {
    "empty.bin": b"",
    "byte42.bin": b"\x42",
    "ascii.bin": b"CompressKit\n",
}

V1_ARCHIVES = {
    "v1/huffman.dat": {
        "algorithm": "huffman",
        "expect": "reject-legacy",
        "payload": b"HFMN" + bytes(32),
    },
    "v1/arithmetic.dat": {
        "algorithm": "arithmetic",
        "expect": "reject-legacy",
        "payload": b"AENC" + bytes(32),
    },
    "v1/range.dat": {
        "algorithm": "range",
        "expect": "reject-legacy",
        "payload": b"RCNC" + bytes(32),
    },
    "v1/rle.dat": {
        "algorithm": "rle",
        "expect": "reject-magic",
        "payload": b"RLE\x00" + bytes(32),
    },
}


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def encode(binary: Path, source: Path, archive: Path) -> None:
    proc = subprocess.run(
        [str(binary), "encode", str(source), str(archive)],
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=TIMEOUT_SECONDS,
        check=False,
    )
    if proc.returncode != 0:
        raise SystemExit(
            f"encode failed: {' '.join(map(str, [binary, source, archive]))}\n"
            f"{proc.stdout}{proc.stderr}"
        )


def main() -> int:
    missing = [a.binary for a in metadata.ALGORITHMS if not a.binary.is_file()]
    if missing:
        raise SystemExit("missing binaries; run `make build` first:\n" + "\n".join(map(str, missing)))

    input_dir = FIXTURE_DIR / "inputs"
    input_dir.mkdir(parents=True, exist_ok=True)
    for name, payload in INPUTS.items():
        (input_dir / name).write_bytes(payload)

    cases: list[dict[str, str]] = []
    for name, payload in INPUTS.items():
        source = input_dir / name
        stem = Path(name).stem
        for algo in metadata.ALGORITHMS:
            archive = FIXTURE_DIR / "v2" / algo.name / f"{stem}.dat"
            archive.parent.mkdir(parents=True, exist_ok=True)
            encode(algo.binary, source, archive)
            cases.append(
                {
                    "id": f"v2-{algo.name}-{stem}",
                    "algorithm": algo.name,
                    "format": "v2",
                    "input": str(source.relative_to(FIXTURE_DIR)),
                    "archive": str(archive.relative_to(FIXTURE_DIR)),
                    "expect": "decode",
                    "input_sha256": sha256_bytes(payload),
                    "archive_sha256": sha256_bytes(archive.read_bytes()),
                }
            )

    for rel, spec in V1_ARCHIVES.items():
        path = FIXTURE_DIR / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(spec["payload"])
        cases.append(
            {
                "id": f"v1-{spec['algorithm']}",
                "algorithm": spec["algorithm"],
                "format": "v1",
                "archive": rel,
                "expect": spec["expect"],
                "archive_sha256": sha256_bytes(spec["payload"]),
            }
        )

    manifest = {
        "version": 1,
        "generator": "tests/fixtures/generate.py",
        "format_generation": "2.0.0",
        "cases": cases,
    }
    (FIXTURE_DIR / "manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )
    print(f"wrote {len(cases)} fixture cases to {FIXTURE_DIR / 'manifest.json'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
