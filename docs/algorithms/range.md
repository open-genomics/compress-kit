# Range Coder

Range Coder 是 Arithmetic 的**整数实现等价物**。它使用整数区间运算而非浮点运算，更适合生产环境，同时实现相同的压缩率。

## 工作原理

Range Coder 使用固定宽度整数维护半开区间 `[low, low + range)`。与 Arithmetic 的位输出不同，Range Coder 输出**字节**，显著提高 I/O 效率。

```cpp
void encode(const vector<uint8_t>& data,
            const vector<uint32_t>& cumFreq) {
    uint32_t low = 0;
    uint32_t range = UINT32_MAX;
    uint32_t total = cumFreq.back();

    for (uint8_t symbol : data) {
        // 重归一化保证 range >= (1 << 24) >= total，因此 r >= 1
        uint32_t r = range / total;
        low += r * cumFreq[symbol];
        range = r * (cumFreq[symbol + 1] - cumFreq[symbol]);

        // 字节级重归一化：顶字节相同则输出；区间过窄且跨越顶字节
        // 边界时，先把区间端点对齐到边界再输出（carryless 处理）
        while ((low ^ (low + range)) < (1u << 24) || range < (1u << 24)) {
            if ((low ^ (low + range)) >= (1u << 24)) {
                range = -low & ((1u << 24) - 1);
            }
            output_byte(low >> 24);
            low <<= 8;
            range <<= 8;
        }
    }
    // 输出最终 4 字节
    flush(low);
}
```

> 关键点：只在「顶字节相同」时移位是不充分的——当区间已经很窄且恰好
> 跨越顶字节边界时，不移位会使 `range` 跌破精度下限，导致后续符号的
> 子区间塌缩为 0，在接近不可压缩的数据上静默丢失数据。因此循环条件
> 必须同时检查 `range` 是否仍低于下限。

## Arithmetic vs Range Coder

| 方面 | Arithmetic | Range Coder |
|------|----------|----------|
| 运算 | 浮点数 | 固定宽度整数 |
| 输出单位 | 位 | 字节 |
| I/O 效率 | 较低 | 较高 |
| 压缩率 | 几乎相同 | 几乎相同 |
| 专利状态 | 历史上有专利 | 无限制 |
| 生产使用 | 学术 | 工业标准 |

## 文件格式

| 字段 | 大小 | 描述 |
|------|------|------|
| 魔数 | 4 字节 | `RCN2` (0x52 0x43 0x4E 0x32) |
| 频率表大小 | 4 字节 | 小端 uint32（始终 257） |
| 频率表 | 257 × 4 字节 | 小端 uint32 数组（符号 0-255 + EOF） |
| 编码数据 | 可变 | 字节流（重归一化区间输出） |
| CRC-32 | 4 字节 | 小端序，覆盖此前全部字节；解码前强制校验 |

通用结构参见 [架构设计 - 二进制格式规范](/architecture/#各算法格式)。

## 复杂度

| 方面 | 复杂度 | 说明 |
|------|--------|------|
| 时间（编码） | O(n) | 与 Arithmetic 类似 |
| 时间（解码） | O(n) | 字节级 I/O 更快 |
| 空间 | O(σ) | 累积频率表 |
| 精度 | 固定 | 32 位整数 |

## 性能特征

| 输入类型 | 压缩率 | 速度 | 内存 |
|----------|--------|------|------|
| 文本 | 1.90× | 58 MiB/s | 低 |

## 适用场景

- ✅ **生产系统** — 最广泛部署的熵编码器
- ✅ **均衡工作负载** — 良好的速度和压缩率
- ✅ **视频编解码器** — H.264、HEVC 使用 Range Coder
- ✅ **压缩工具** — 用于现代归档工具

## 命令行使用

```bash
./build/rangecoder_cpp encode input.bin output.rcnc
./build/rangecoder_cpp decode output.rcnc restored.bin
```

## 延伸阅读

- [Arithmetic](/algorithms/arithmetic) — 同一熵编码思想的位级实现
- [区间编码学院](/academy/range) — 精度下限与 v2 修复
- [基准测试](/benchmarks/results) — `make bench` 在 10 MiB 语料上的快照
