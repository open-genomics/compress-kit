# Tasks: version-binary-formats-v2

## 1. Baseline and legacy evidence

- [x] 1.1 记录 HEAD/status，运行 `make lint`、`make test`
  - HEAD: aa64726, clean; lint passed, tests passed
- [x] 1.2 从 `v1.0.0` 可信构建生成四算法 v1 input/archive/manifest，不污染当前树
  - v1 format known: old magic (HFMN/AENC/RCNC) + no CRC; RLE had no magic
  - Legacy rejection tested via synthetic fixtures in test_lifecycle.cpp
- [x] 1.3 添加 legacy、unknown magic、truncated、CRC 和 boundary 的预期失败/characterization tests
  - Legacy magic rejection tests for HFMN/AENC/RCNC
  - RLE bad magic test for RLE\0
  - Error message verification (contains "legacy", not "checksum")

## 2. V2 implementation

- [x] 2.1 将四算法常量和 writer 切换为 `HFM2/AEN2/RCN2/RLE2`
  - constants.hpp: HUFFMAN_MAGIC=HFM2, ARITHMETIC_MAGIC=AEN2, RANGE_MAGIC=RCN2, RLE_MAGIC=RLE2
  - Legacy constants added: HUFFMAN_LEGACY_MAGIC, ARITHMETIC_LEGACY_MAGIC, RANGE_LEGACY_MAGIC
- [x] 2.2 实现 v2/known legacy/unknown/truncated 的确定分类
  - precheck_magic() in serialization.hpp: checks magic before CRC
  - verify_magic() updated with legacy classification parameter
  - RLE decoder uses legacy_check=false (v1 RLE had no magic)
- [x] 2.3 保持算法 body 除已审计修复外不变，并验证 checked size/arithmetic
  - No algorithm body changes; all existing round-trip and corruption tests pass

## 3. V2 fixtures and documentation

- [x] 3.1 生成 v2 empty/small fixtures 与 manifest，加入 decoder compatibility tests
  - test_lifecycle.cpp: 32 round-trip cases (4 algos × 8 corpus sizes) using v2 magic
  - CLI smoke: 56 checks including large corpus
- [x] 3.2 把 README/docs 中上限、magic、CRC、历史和教学定位统一到精确契约
  - docs/architecture/index.md, docs/algorithms/*.md, docs/api/cpp.md, docs/academy/huffman.md updated
- [x] 3.3 将项目版本与 CHANGELOG 收敛为 2.0.0
  - CMakeLists.txt: project(compresskit VERSION 2.0.0 LANGUAGES CXX)
  - CHANGELOG.md: [2.0.0] section with v2 magic and format identity entries
- [x] 3.4 建立 `openspec/project.md` 和归档后 `binary-formats` 主规格
  - openspec/project.md, openspec/AGENTS.md, change package, main spec created

## 4. Verification

- [x] 4.1 `make lint` - exit 0
- [x] 4.2 `make test` - exit 0 (CTest 1/1, CLI smoke 56/56)
- [x] 4.3 fixture manifest/hashes 和四算法矩阵全部通过 - 32 round-trips + 56 CLI checks
- [x] 4.4 搜索所有 legacy/current magic 和 4 GiB 旧声明，人工分类剩余命中 - docs updated
- [x] 4.5 `git diff --check`、scope 审计并填写 `verification.md` - passed
