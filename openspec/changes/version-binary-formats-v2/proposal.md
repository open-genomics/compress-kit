# Change Proposal: version-binary-formats-v2

## Metadata

- Status: `Applying`
- Repository: `open-genomics/compress-kit`
- Audit base: `aa6472604fe25e567fa4fe56f31c3baf2b3577a6`
- Capability: `binary-formats`
- Task IDs: `CK-DOC-001`, `CK-FMT-001`, `CK-TEST-001`, `ORG-GOV-001`, `ORG-CONTRACT-001`
- Decision IDs: `CK-DEC-001`

## Why

当前 Unreleased 格式相对 `v1.0.0` 已增加 CRC trailer、修复 Range bitstream 并改变 RLE framing，却继续复用部分 v1 magic。decoder 无法可靠区分旧文件和新文件。格式身份、fixture、文档和发布 major 必须作为一个 change 闭环。

## Changes

**Four binary format identities**

- From: Huffman/Arithmetic/Range 沿用旧 magic，RLE 处于未发布中间 magic，旧 payload 常被误报 checksum 错误。
- To: v2 使用 `HFM2/AEN2/RCN2/RLE2`；writer 只写 v2；可识别 v1 明确报 legacy unsupported；包版本按 `2.0.0` 收敛。
- Reason: 在解析 body 前确定格式世代。
- Impact: breaking format release；不提供 v1 reader；必须保留 v1 fixtures 证明稳定拒绝。

## Scope

- 四算法 magic、decoder 错误分类、格式测试和 fixtures；
- 精确的 CRC/endian/size 文档；
- CMake/单一版本来源、CHANGELOG 的 2.0.0 语义；
- 仓库内 `binary-formats` 主规格基础。

## Out of scope

- 不实现 v1 read-only decoder；
- 不改变 CLI 形状或压缩算法；
- 不实施 streaming、分配前文件检查或原子输出；
- 不支持未发布的 CRC + legacy magic 中间格式。
- 不恢复仓库曾删除的 Node、编辑器 skills 或完整 OpenSpec 元工具；只提交本 change 所需的纯 Markdown artifacts。

## Compatibility and rollback

这是有意的 major 格式边界。已经产生的未发布中间文件没有稳定承诺；v1 保留为 fixture 并确定拒绝。任何合并状态都必须同时包含 writer、reader、tests、docs 和版本语义，不能只回滚其中一部分。

## Approval

- `CK-DEC-001`: Accepted on 2026-08-13
- Exact values: `HFM2/AEN2/RCN2/RLE2`, reject legacy, release `2.0.0`
- Apply approval: `authorized by organization owner`
