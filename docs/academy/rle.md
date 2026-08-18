---
title: 行程编码深度解析
description: 最简单的无损变换及其膨胀上界
---

# 行程编码深度解析

行程编码（RLE）把连续相同字节替换为 `(count, value)`。它没有熵模型，最坏情况下会膨胀，最好情况下对长重复极其有效。

## 规则

输入 `AAAABB` 变成两段：`(4, 'A')`、`(2, 'B')`。本实现中 `count` 是小端 `uint32`，`value` 是 1 字节，每段 5 字节。

无重复时每个输入字节变成 5 字节，**膨胀约 5×**。这就是压缩输入上限（8 GiB）大于原始数据上限（1 GiB）的原因：一份不可压缩的 1 GiB 输入，RLE 流可能接近 5 GiB。

`count` 必须大于 0；解码时若 `count==0` 或累加输出将超过 1 GiB，一律拒绝。

## 本仓库的格式

```
| RLE2 | (count:u32 LE, value:u8)* | CRC-32 |
```

v1 RLE **没有 magic**。当前解码器对非 `RLE2` 前缀报 `bad magic`，不会去猜「这是不是旧行程对」。

空输入合法：只写 magic + CRC。

## 什么时候用

- 位图、掩码、大量连续 0/1
- 作为其他变换（BWT 等）之后的简单后处理——本仓库没有 BWT，RLE 是单独算法
- **不要**对已经压缩或接近随机的数据使用，除非你在演示膨胀

```bash
make stats
# repetitive_10MiB 上 RLE 的 ratio 会远小于 1
# random_1MiB 上 ratio 会接近 5
python3 tests/lab_stats.py tests/data/repetitive_10MiB.bin
python3 tests/lab_stats.py tests/data/random_1MiB.bin
```

## 复杂度

时间 O(n)，额外空间 O(1)（输出缓冲除外）。没有树、没有频率表。教学价值在于：看见「简单规则」如何既可能压缩 100×，也可能膨胀 5×。

## 延伸阅读

- [RLE 算法页](/algorithms/rle) — 行程对布局与最坏情况警告
- [算法选择](/guide/algorithms) — 高度重复时优先 RLE
- [架构](/architecture/) — 1 GiB / 8 GiB 体积契约
