#!/usr/bin/env python3
"""Unit checks for Shannon entropy used by the lab diagnostic."""

from __future__ import annotations

import math
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from lab_stats import shannon_entropy_bits_per_byte


def check(cond: bool, message: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL {message}")


def main() -> int:
    check(shannon_entropy_bits_per_byte(b"") == 0.0, "empty input entropy should be 0")
    check(shannon_entropy_bits_per_byte(b"AAAA") == 0.0, "constant input entropy should be 0")
    two = shannon_entropy_bits_per_byte(b"ABAB")
    check(abs(two - 1.0) < 1e-12, f"balanced two-symbol entropy should be 1, got {two}")
    uniform = bytes(range(256)) * 4
    got = shannon_entropy_bits_per_byte(uniform)
    check(abs(got - 8.0) < 1e-12, f"uniform byte entropy should be 8, got {got}")
    check(math.isfinite(got), "entropy must be finite")
    print("PASS lab_stats entropy")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
