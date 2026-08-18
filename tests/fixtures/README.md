# Frozen format fixtures

Committed archives lock the v2 decoder and v1 rejection contract.

- `inputs/` — canonical raw payloads
- `v2/<algorithm>/` — streams produced by the v2 encoder
- `v1/` — synthetic legacy magics that must be rejected
- `manifest.json` — SHA-256 hashes and expected decode/reject results

Regenerate after an intentional format change:

```bash
make build
python3 tests/fixtures/generate.py
python3 tests/conformance/run_fixtures.py
```

Do not regenerate to “make CI green” after an accidental bitstream change.
