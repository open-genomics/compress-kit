# CompressKit Agent Guide

## Project Identity

- Product: **CompressKit** - C++17 compression laboratory
- Repository: `AICL-Lab/compress-kit`
- Default branch: `master`

## Core Contract

Four algorithms (Huffman, Arithmetic, Range, RLE) implemented in C++17.
Binary format compatibility is the primary constraint.

**Magic Numbers**:
| Algorithm | Magic |
|-----------|-------|
| Huffman | `HFMN` |
| Arithmetic | `AENC` |
| Range Coder | `RCNC` |
| RLE | `RLE\x00` |

Every stream ends with a little-endian CRC-32 trailer (4 bytes) covering all
preceding bytes; decoders verify it before parsing.

## Validation Commands

| Command | Purpose |
|---------|---------|
| `make build` | Build all C++ CLIs (CMake) |
| `make test` | Unit tests + CLI smoke tests |
| `make lint` | clang-format dry-run |

## Key Constraints

- Maintain binary format compatibility (magic bytes, frequency table LE layout, RLE pair layout, CRC-32 trailer)
- Size limits: raw data (encode input / decode output) strictly below 1 GiB; compressed decode input strictly below 8 GiB
- Error messages in code must be English
- User-facing documentation (README, docs/) must be in Simplified Chinese

## Change Policy

Binary format changes and new algorithms require careful design review.
Internal refactors and bug fixes that preserve existing contract may be
implemented directly.
