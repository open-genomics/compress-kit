---
title: 算术编码
description: 区间划分的熵编码、实现与文件格式
---

# 算术编码

算术编码把**整条消息**映射到半开区间 `[0, 1)` 中的一个数值，而非为每个符号
分配整数比特的码字。平均码长可逼近信息熵 `L ≈ H + ε`，不受 Huffman
「每符号至少 1 比特」的限制。

## 工作原理

设符号 `s` 的概率为 `p_s`、累积概率为 `F(s)`。当前区间 `[low, high)` 编码 `s`
后收缩为 `[low + (high-low)F(s), low + (high-low)F(s+p_s))`。消息越长区间越窄，
所需比特数约为 `-log2(high-low)`。

> 下方伪代码用浮点演示原理；本仓库的 C++17 实现使用固定宽度整数、缩放频率表
> 与位级重归一化，避免浮点精度耗尽。

```cpp
double low = 0.0, high = 1.0;
for (uint8_t s : data) {
    double range = high - low;
    high = low + range * cumProb[s + 1];
    low  = low + range * cumProb[s];
}
```

## 本仓库实现

- **静态模型**：先扫描全文得到 256 字节频率，再加 EOF 符号，编码期间不更新。
- **频率缩放**：总和限制在 `2^24`，保证 `range / total` 不会把子区间压成 0。
- **EOF**：解码器读到 EOF 符号后停止，不依赖外部长度字段。
- **整数运算**：固定宽度整数 + 位级重归一化，周期性输出位以避免下溢。
- v1 magic `AENC` 被明确拒绝。

## 文件格式

| 字段 | 大小 | 描述 |
|------|------|------|
| Magic | 4 字节 | `AEN2` |
| 频率表大小 | 4 字节 | 小端 uint32（始终 257） |
| 频率表 | 257 × 4 字节 | 小端 uint32 数组（符号 0-255 + EOF） |
| 编码数据 | 可变 | 位流（重归一化区间输出） |
| CRC-32 | 4 字节 | 小端序，覆盖此前全部字节；解码前强制校验 |

## 命令行用法

```bash
./build/arithmetic_cpp encode input.bin output.aenc
./build/arithmetic_cpp decode output.aenc restored.bin
```

## 复杂度

| 方面 | 复杂度 | 说明 |
|------|--------|------|
| 时间（编码/解码） | O(n) | 单次遍历，区间更新 |
| 空间 | O(σ) | 概率表 |
| 精度 | 固定 | 整数运算 + 重归一化 |

## 注意事项

- 偏斜分布上与 Huffman 压缩率接近；接近均匀分布时优势更明显。
- 仍需为频率表付出约 1 KiB 固定开销，对小文件不划算。
- 用 `make stats` 可对照 Shannon 熵与实际码长。
- 详见 [基准测试](/benchmarks/results) 与 [架构概览](/architecture/)。
