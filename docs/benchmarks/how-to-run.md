# 如何运行基准测试

基准快照位于 `docs/.vitepress/data/benchmarks.json`，文档站图表直接读取该文件。

## 一键重跑

```bash
make bench
```

这会构建 CLI、生成语料，对每个算法 × 数据集做 encode/decode 计时，并覆盖 JSON 快照。随后：

```bash
cd docs && npm ci && npm test && npm run build
```

## 当前数据集

| 文件 | 用途 |
|------|------|
| `textlike_10MiB.bin` | 四算法主对比 |
| `repetitive_10MiB.bin` | RLE 有意义的高重复输入；其他算法仍跑，便于对照 |
| `random_1MiB.bin` | 近不可压缩输入（RLE 会膨胀） |

四个算法都在同一文件上计时。Range Coder 已能 round-trip 10 MiB；不要再用 8 KiB 样本代替它。

## 测量指标

| 指标 | 描述 |
|------|------|
| 编码时间 | 压缩输入的挂钟时间（毫秒） |
| 解码时间 | 恢复原始的挂钟时间（毫秒） |
| 编码/解码速度 | MiB/s = 输入大小 / 时间 |
| 压缩比 | 输出大小 / 输入大小（越小越好） |

## 熵对照

```bash
make stats
python3 tests/lab_stats.py tests/data/repetitive_10MiB.bin
```

输出输入的 Shannon 熵（比特/字节）以及每种算法压缩后的实际比特/字节。

## 故障排除

二进制或语料缺失时，先运行 `make build` 与 `make test-data`。
