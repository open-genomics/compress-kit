# 基准测试结果

交互式图表读取仓库内的 `docs/.vitepress/data/benchmarks.json`。数字以该快照为准；换机器后请 `make bench` 刷新。

## 交互式性能图表

<BenchmarkChart />

## 生成数据模型

| JSON 位置 | 字段 | 含义 |
|----------|------|------|
| 顶层 | `generated` | 快照日期 |
| 顶层 | `version` | 对应的项目版本 |
| `results[]` | `algorithm`、`dataset` | 图表坐标 |
| `results[]` | `encodeTime`、`decodeTime` | 挂钟时间，毫秒 |
| `results[]` | `encodeSpeed`、`decodeSpeed` | 吞吐量，MiB/s |
| `results[]` | `compressionRatio`、`throughput` | 输出/输入比值，以及粗粒度吞吐标签 |

## 当前数据集

| 数据集键 | 图表标签 |
|----------|----------|
| `textlike_10MiB` | 类文本 (10 MiB) |
| `repetitive_10MiB` | 重复数据 (10 MiB) |
| `random_1MiB` | 随机 (1 MiB) |

四个算法使用同一组文件。RLE 在随机数据上会膨胀（比值大于 1）；这是格式契约，不是测量错误。

## 刷新

```bash
make bench
cd docs && npm run build
```

提交时带上生成的 JSON，不要手改本页里的数字。

## 另见

- [如何运行基准测试](/benchmarks/how-to-run)
- [算法指南](/guide/algorithms)
- [熵对照](/academy/) — `make stats`
