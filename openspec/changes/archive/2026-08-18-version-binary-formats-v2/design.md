# Design: version-binary-formats-v2

## Approach

1. 从 tag `v1.0.0` 的可信构建生成每算法 v1 fixture 和 manifest；
2. 先写新 magic、legacy/unknown/truncated/CRC 的失败测试；
3. 将常量和 writer/parser 切换到已批准 v2 magic；
4. decoder 在读取前 4 bytes 后分类，不以 CRC 猜版本；
5. 生成 v2 empty/small fixtures，并冻结 reader compatibility；
6. 把 CMake、README、docs 和 CHANGELOG 统一到 v2/2.0.0。

## Wire identities

| Algorithm | Legacy | v2 |
|---|---|---|
| Huffman | `HFMN` | `HFM2` |
| Arithmetic | `AENC` | `AEN2` |
| Range | `RCNC` | `RCN2` |
| RLE | v1 no magic; unpublished `RLE\0` | `RLE2` |

v2 通用结构为 `magic + algorithm body + crc32:u32 little-endian`，CRC 覆盖 trailer 之前全部 bytes。magic/CRC 不是认证机制。

## Decoder classification

- v2 magic：校验最小长度和 CRC，再解析 body；
- `HFMN/AENC/RCNC`：legacy unsupported；
- RLE v1 因无 magic 不可自动识别：bad magic expected RLE2；
- `RLE\0` 中间格式：unsupported unpublished format 或明确 bad magic，不当作稳定 v1/v2；
- 其他：bad magic；
- 任何路径在 checked size/arithmetic 失败时停止。

## Allowed surface

- `algorithms/shared/cpp/include/compresskit/constants.hpp`
- 四算法 serializer/parser 和共享 CRC/error code
- lifecycle/CLI conformance tests、`tests/fixtures/`
- CMake/VERSION、README、docs、CHANGELOG
- `openspec/`

仓库 changelog 中“删除旧 OpenSpec/编辑器元工具”的历史保持不变。本 change 的 `openspec/` 是无 Node/CLI 依赖的纯 Markdown 格式契约，不恢复被清理的外围工具。

## Fixture manifest

每项记录 algorithm、format version、source commit/tag、generator command、input/archive hashes、expected decode/reject。v2 writer 若不承诺 canonical bytes，只比较稳定结构；frozen v2 archive 必须持续可读。

## Risk and rollback

风险是历史 fixture 不可重建或错误标记 v1。生成过程必须使用 detached worktree/临时构建且不改当前树，并保存 source SHA。若任一算法的 v1 layout 无法确认，阻塞该算法而不是猜测。
