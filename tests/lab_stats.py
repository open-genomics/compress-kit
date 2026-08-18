#!/usr/bin/env python3
"""Compare Shannon entropy of a file against each algorithm's compressed size."""

from __future__ import annotations

import argparse
import math
import subprocess
import sys
import tempfile
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import metadata

TIMEOUT_SECONDS = 30.0


def shannon_entropy_bits_per_byte(data: bytes) -> float:
    if not data:
        return 0.0
    counts = Counter(data)
    n = len(data)
    return -sum((c / n) * math.log2(c / n) for c in counts.values())


def _run(command: list[str]) -> None:
    proc = subprocess.run(
        command,
        cwd=metadata.ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=TIMEOUT_SECONDS,
        check=False,
    )
    if proc.returncode != 0:
        raise RuntimeError(
            f"command failed (exit {proc.returncode}): {' '.join(command)}\n"
            f"{proc.stdout}{proc.stderr}"
        )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path, help="raw input file")
    args = parser.parse_args(argv)

    source = args.input
    data = source.read_bytes()
    entropy = shannon_entropy_bits_per_byte(data)
    print(f"input: {source}  bytes={len(data)}  entropy={entropy:.4f} bits/byte")
    if not data:
        print("empty input: entropy bound is 0; encoders still emit headers + CRC")

    missing = [a.binary for a in metadata.ALGORITHMS if not a.binary.is_file()]
    if missing:
        raise SystemExit("missing binaries; run `make build` first:\n" + "\n".join(map(str, missing)))

    with tempfile.TemporaryDirectory(prefix=".lab-stats-", dir=metadata.TESTS_DIR) as tmp:
        tmpdir = Path(tmp)
        print(f"{'algorithm':<12} {'out':>10} {'ratio':>8} {'bits/byte':>10} {'vs entropy':>11}")
        for algo in metadata.ALGORITHMS:
            encoded = tmpdir / f"{algo.name}.dat"
            _run([str(algo.binary), "encode", str(source), str(encoded)])
            out_size = encoded.stat().st_size
            ratio = out_size / len(data) if data else float("inf")
            bits = (out_size * 8) / len(data) if data else float("inf")
            delta = bits - entropy if data else float("inf")
            print(
                f"{algo.name:<12} {out_size:>10} {ratio:>8.4f} {bits:>10.4f} {delta:>+11.4f}"
            )
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
