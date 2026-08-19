---
title: RLE 行程编码
description: 行程对的简单变换、膨胀上界与文件格式
---

# RLE 行程编码

RLE（Run-Length Encoding）把连续相同字节替换为 `(count, value)` 对。它没有
熵模型，速度极快，对长连续重复数据效果极好；但对不可压缩数据会膨胀。

## 工作原理

输入 `AAAABB` 变成两段：`(4, 'A')`、`(2, 'B')`。本实现中 `count` 是小端
`uint32`，`value` 是 1 字节，每段 5 字节。

```cpp
for (size_t i = 0; i < data.size();) {
    uint8_t cur = data[i];
    uint32_t count = 1;
    while (i + count < data.size() && data[i + count] == cur && count < UINT32_MAX)
        ++count;
    write_u32_le(count);   // 4 字节小端
    write_u8(cur);         // 1 字节
    i += count;
}
```

## 本仓库实现

- 无频率表、无树，额外空间 O(1)（输出缓冲除外）。
- `count` 必须大于 0；解码时若 `count == 0` 或累加输出将超过 1 GiB，一律拒绝。
- 空输入合法：只写 magic + CRC。
- v1 RLE **没有 magic**；当前解码器对非 `RLE2` 前缀报 `bad magic`，不会去猜
  旧行程对格式。

## 文件格式

| 字段 | 大小 | 描述 |
|------|------|------|
| Magic | 4 字节 | `RLE2` (0x52 0x4C 0x45 0x32) |
| 行程对 0 | 5 字节 | `count` (4 字节小端 uint32) + `value` (1 字节) |
| 行程对 1 | 5 字节 | 同上 |
| ... | ... | 重复直到编码数据结束 |
| CRC-32 | 4 字节 | 小端序，覆盖此前全部字节；解码前强制校验 |

## 命令行用法

```bash
./build/rle_cpp encode input.bin output.rle
./build/rle_cpp decode output.rle restored.bin
```

## 复杂度

| 方面 | 复杂度 | 说明 |
|------|--------|------|
| 时间（编码/解码） | O(n) | 单次遍历 |
| 空间 | O(1) | 无辅助结构 |

## 注意事项

- **最坏膨胀约 5×**：无重复时每个输入字节变成 5 字节。这正是压缩输入上限
  （8 GiB）大于原始数据上限（1 GiB）的原因——一份不可压缩的 1 GiB 输入，
  RLE 流可能接近 5 GiB。
- 适合位图、掩码、大量连续 0/1 等长重复数据。
- **不要**对已压缩或接近随机的数据使用，除非在演示膨胀。
- 用 `make stats` 可在 `repetitive_10MiB`（ratio 远小于 1）与 `random_1MiB`
  （ratio 接近 5）上对照。
- 详见 [基准测试](/benchmarks/results) 与 [架构概览](/architecture/)。
