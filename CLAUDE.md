# Claude Instructions for CompressKit

Follow `AGENTS.md` first. This file adds Claude-specific reminders.

## Quick Validation

```bash
make test && make lint
```

## Compression Guardrails

- Do not change magic bytes, frequency table layout, endian rules, or RLE pair layout
- RLE magic is `RLE2`
- Keep security limits: raw data (encode input / decode output) strictly below 1 GiB; compressed decode input strictly below 8 GiB

## Documentation Stance

- README: 项目入口（中文）
- Changelog: User-facing changes only
