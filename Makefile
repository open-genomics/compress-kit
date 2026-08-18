.PHONY: build test test-data test-cli-smoke sanitize bench stats clean format lint help

# ── Build ──────────────────────────────────────────────────────────────────

build:
	cmake -S . -B build
	cmake --build build

# ── Test ───────────────────────────────────────────────────────────────────

test: test-data build
	ctest --test-dir build --output-on-failure
	$(MAKE) test-cli-smoke
	python3 tests/conformance/run_fixtures.py
	python3 tests/test_lab_stats.py

test-cli-smoke: test-data build
	python3 tests/conformance/run_cli_smoke.py

SANITIZE_FLAGS := -fsanitize=address,undefined -fno-omit-frame-pointer

sanitize:
	cmake -S . -B build-san -DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_FLAGS="$(SANITIZE_FLAGS)" \
		-DCMAKE_EXE_LINKER_FLAGS="$(SANITIZE_FLAGS)"
	cmake --build build-san
	ctest --test-dir build-san --output-on-failure

bench: test-data build
	python3 tests/bench.py

stats: test-data build
	python3 tests/lab_stats.py tests/data/textlike_10MiB.bin

# ── Data / Clean ───────────────────────────────────────────────────────────

test-data:
	python3 tests/gen_testdata.py

clean:
	rm -rf build build-san tests/data

# ── Format & Lint ───────────────────────────────────────────────────────────

format:
	@command -v clang-format >/dev/null || { echo "clang-format is required"; exit 1; }
	@echo "Formatting C++ code..."
	@find algorithms -type f \( -name '*.cpp' -o -name '*.hpp' \) -exec clang-format -i {} +
	@echo "Done!"

lint:
	@command -v clang-format >/dev/null || { echo "clang-format is required"; exit 1; }
	@echo "Linting C++ code (clang-format dry-run)..."
	@find algorithms -type f \( -name '*.cpp' -o -name '*.hpp' \) \
		! -path '*/build/*' \
		! -path '*/build-san/*' \
		-exec clang-format --dry-run -Werror {} + || { echo "clang-format check failed; run 'make format'"; exit 1; }
	@echo "Done!"

# ── Help ────────────────────────────────────────────────────────────────────

help:
	@echo "Build Commands:"
	@echo "  make build          Build all algorithms (CMake)"
	@echo ""
	@echo "Test Commands:"
	@echo "  make test           Run all tests + CLI smoke + fixtures"
	@echo "  make test-cli-smoke Run CLI smoke tests"
	@echo "  make sanitize       Build with ASan/UBSan and run lifecycle tests"
	@echo "  make bench          Refresh docs/.vitepress/data/benchmarks.json"
	@echo "  make stats          Print entropy vs compressed size for textlike_10MiB"
	@echo ""
	@echo "Code Quality:"
	@echo "  make format         Format all C++ code"
	@echo "  make lint           Lint all C++ code (clang-format dry-run)"
	@echo ""
	@echo "Other:"
	@echo "  make clean          Clean build artifacts"
	@echo "  make test-data      Generate test corpus"
