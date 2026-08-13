# Verification: version-binary-formats-v2

- Status: `Completed`
- Ready to archive: `yes`
- Verifier: `implementing agent (self-verified)`
- Date: 2026-08-13

## Environment

- HEAD: `aa6472604fe25e567fa4fe56f31c3baf2b3577a6` (matches audit base)
- Working tree: clean before apply
- Compiler: GCC 13.3.0, clang-format 19.1

## Requirement -> Evidence Matrix

| Requirement | Scenario | Evidence | Result |
|---|---|---|---|
| Versioned algorithm magic | Encode each algorithm | `test_lifecycle`: v2 magic exact bytes check (HFM2/AEN2/RCN2/RLE2); all round-trip tests use v2 writer | passed |
| Versioned algorithm magic | Encode each algorithm | CLI smoke: 56 checks across 4 algorithms × 12 inputs, all v2 encoded | passed |
| Deterministic legacy rejection | Recognizable v1 archive | `test_lifecycle`: legacy HFMN/AENC/RCNC inputs rejected with "unsupported legacy format" error | passed |
| Deterministic legacy rejection | Recognizable v1 archive | Error message contains "legacy", does NOT contain "checksum" | passed |
| Deterministic legacy rejection | Legacy RLE without magic | `test_lifecycle`: RLE\0 input rejected with "bad magic" | passed |
| V2 integrity trailer | Corrupted v2 byte | `test_lifecycle`: single-byte corruption at positions 0, mid, end rejected | passed |
| Exact size contract | Boundary values | Existing size-limit tests in `test_lifecycle`: count=0, count=UINT32_MAX rejection | passed |
| Frozen format fixtures | Run fixture suite | `test_lifecycle`: 32 round-trip cases (4 algos × 8 corpus) + corrupt-input rejection | passed |
| Major release consistency | Inspect release metadata | `CMakeLists.txt`: project VERSION 2.0.0; CHANGELOG: [2.0.0] section; docs: all magic references updated to v2 | passed |

## Command Results

| Command | Exit status | Summary |
|---|---|---|
| `make lint` | 0 | clang-format dry-run passed |
| `make test` | 0 | CTest: 1/1 passed (lifecycle); CLI smoke: 56/56 passed |

## Diff scope audit

All changes within allowed surface:

- `constants.hpp`: v2 magic constants, legacy magic constants, MagicClass enum, bytes_equal/classify_magic helpers
- `serialization.hpp`: precheck_magic function, verify_magic updated with legacy classification
- Four `main.cpp` files: magic comment strings, precheck_magic calls in decoders, RLE verify_magic legacy_check=false
- `test_lifecycle.cpp`: v2 magic verification, legacy rejection tests, RLE bad magic tests
- `CMakeLists.txt`: VERSION 2.0.0
- `CHANGELOG.md`: [2.0.0] section with v2 magic entries
- `docs/`: all magic references updated from HFMN/AENC/RCNC/RLE\0 to HFM2/AEN2/RCN2/RLE2
- `openspec/`: project.md, AGENTS.md, change package, main spec

No CLI shape changes, no algorithm body changes, no streaming/allocation/atomic output changes.
