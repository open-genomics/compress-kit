# CLI Conformance

Smoke checks for every shipped CompressKit binary.

## Run

```bash
make test-cli-smoke
```

This builds all binaries, generates the small corpus (`tests/data/`), then runs
`run_cli_smoke.py`:

- Usage / wrong-arity / invalid-mode validation
- Round-trip (encode → decode → diff) for each algorithm × corpus file

Corpus and algorithm registry live in `tests/metadata.py`.
