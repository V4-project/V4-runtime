.PHONY: all build release test clean format format-check asan ubsan esp32c6 size help

# Default target
all: build test

help:
	@echo "V4 RTOS Build System"
	@echo ""
	@echo "Targets:"
	@echo "  build         - Build all components (debug)"
	@echo "  release       - Build release version"
	@echo "  test          - Run all tests"
	@echo "  clean         - Clean build artifacts"
	@echo "  format        - Format all source code"
	@echo "  format-check  - Check code formatting"
	@echo "  asan          - Build and test with AddressSanitizer"
	@echo "  ubsan         - Build and test with UndefinedBehaviorSanitizer"
	@echo "  esp32c6       - Build ESP32-C6 runtime"
	@echo "  size          - Show firmware sizes for all BSPs"
	@echo ""
	@echo "Variables:"
	@echo "  DOCKER=1      - Use Docker for ESP32-C6 build"
	@echo "                  Example: make esp32c6 DOCKER=1"
	@echo ""

# Build (default: debug, override with CMAKE_BUILD_TYPE=Release)
BUILD_TYPE ?= Debug

build:
	@echo "🔨 Building V4 RTOS ($(BUILD_TYPE))..."
	@cmake -B build -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DV4_BUILD_KERNEL=ON -DV4_BUILD_TESTS=ON $(if $(V4_FETCH),-DV4_FETCH=ON,)
	@cmake --build build -j
	@echo "✅ Build complete!"

# Release build
release:
	@echo "🚀 Building V4 RTOS (release)..."
	@cmake -B build-release -DCMAKE_BUILD_TYPE=Release
	@cmake --build build-release -j
	@echo "✅ Release build complete!"

# Run tests
test: build
	@echo "🧪 Running tests..."
	@cd build && ctest --output-on-failure
	@echo "✅ Tests complete!"

# Clean
clean:
	@echo "🧹 Cleaning..."
	@rm -rf build build-release build-debug build-asan build-ubsan
	@echo "✅ Clean complete!"

# Apply formatting
format:
	@echo "✨ Formatting C/C++ code..."
	@find kernel hal compiler shell protocol bsp -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' -o -name '*.c' \) \
		-not -path "*/build/*" -not -path "*/vendor/*" -exec clang-format -i {} \; 2>/dev/null || true
	@echo "✨ Formatting CMake files..."
	@find . -name 'CMakeLists.txt' -o -name '*.cmake' | grep -v build | xargs cmake-format -i 2>/dev/null || true
	@echo "✅ Formatting complete!"

# Format check
format-check:
	@echo "🔍 Checking C/C++ formatting..."
	@find kernel hal compiler shell protocol bsp -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' -o -name '*.c' \) \
		-not -path "*/build/*" -not -path "*/vendor/*" | xargs clang-format --dry-run --Werror 2>/dev/null || \
		(echo "❌ C/C++ formatting check failed. Run 'make format' to fix." && exit 1)
	@echo "🔍 Checking CMake formatting..."
	@find . -name 'CMakeLists.txt' -o -name '*.cmake' | grep -v build | xargs cmake-format --check 2>/dev/null || \
		(echo "❌ CMake formatting check failed. Run 'make format' to fix." && exit 1)
	@echo "✅ All formatting checks passed!"

# Sanitizer build
asan: clean
	@echo "🛡️  Building with AddressSanitizer..."
	@cmake -B build-asan -DCMAKE_BUILD_TYPE=Debug -DV4_BUILD_TESTS=ON \
		-DCMAKE_C_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g" \
		-DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g"
	@cmake --build build-asan -j
	@echo "🧪 Running tests with AddressSanitizer..."
	@cd build-asan && ctest --output-on-failure
	@echo "✅ ASAN tests complete!"

ubsan: clean
	@echo "🛡️  Building with UndefinedBehaviorSanitizer..."
	@cmake -B build-ubsan -DCMAKE_BUILD_TYPE=Debug -DV4_BUILD_TESTS=ON \
		-DCMAKE_C_FLAGS="-fsanitize=undefined -fno-omit-frame-pointer -g" \
		-DCMAKE_CXX_FLAGS="-fsanitize=undefined -fno-omit-frame-pointer -g"
	@cmake --build build-ubsan -j
	@echo "🧪 Running tests with UndefinedBehaviorSanitizer..."
	@cd build-ubsan && ctest --output-on-failure
	@echo "✅ UBSAN tests complete!"

# Build ESP32-C6 runtime
esp32c6:
ifeq ($(DOCKER),1)
	@echo "📱 Building ESP32-C6 runtime (Docker)..."
	@if ! command -v docker >/dev/null 2>&1; then \
		echo "❌ Docker not found. Please install Docker first."; \
		exit 1; \
	fi
	@docker compose run --rm esp-idf bash -c "git config --global --add safe.directory /project && idf.py build"
	@echo "✅ ESP32-C6 runtime build complete!"
	@echo ""
	@echo "To flash (Docker):"
	@echo "  docker compose run --rm esp-idf bash -c 'git config --global --add safe.directory /project && idf.py flash monitor'"
else
	@echo "📱 Building ESP32-C6 runtime (native)..."
	@if [ -z "$$IDF_PATH" ]; then \
		echo "❌ ESP-IDF not found. Please either:"; \
		echo "   1. Source ESP-IDF: . $$HOME/esp/esp-idf/export.sh"; \
		echo "   2. Use Docker: make esp32c6 DOCKER=1"; \
		exit 1; \
	fi
	@cd bsp/esp32c6/runtime && idf.py build
	@echo "✅ ESP32-C6 runtime build complete!"
	@echo ""
	@echo "To flash:"
	@echo "  cd bsp/esp32c6/runtime && idf.py flash monitor"
endif

# Show firmware sizes for all BSPs
size:
	@echo "📦 Firmware Size Report"
	@echo "======================="
	@echo ""
	@echo "=== ESP32-C6 (M5Stack NanoC6) ==="
	@if [ -f bsp/esp32c6/runtime/build/v4-runtime.elf ]; then \
		echo ""; \
		echo "Memory usage:"; \
		riscv32-esp-elf-size bsp/esp32c6/runtime/build/v4-runtime.elf 2>/dev/null || \
		$$IDF_PATH/tools/riscv32-esp-elf/*/riscv32-esp-elf/bin/riscv32-esp-elf-size bsp/esp32c6/runtime/build/v4-runtime.elf 2>/dev/null || \
		echo "  ⚠️  riscv32-esp-elf-size not found in PATH"; \
		echo ""; \
		echo "Firmware binaries:"; \
		for bin in bsp/esp32c6/runtime/build/*.bin; do \
			if [ -f "$$bin" ]; then \
				printf "  %-30s %s\n" "$$(basename $$bin):" "$$(ls -lh $$bin | awk '{print $$5}')"; \
			fi \
		done; \
		echo ""; \
		echo "Partition table:"; \
		cat bsp/esp32c6/runtime/build/partition_table/partition-table.csv 2>/dev/null | \
			awk 'NR==1 || /^[^#]/ {printf "  %s\n", $$0}' || echo "  ⚠️  Partition table not found"; \
	else \
		echo "  ⚠️  Build not found. Run 'make esp32c6' first."; \
	fi
	@echo ""
	@echo "To build before checking size:"
	@echo "  make esp32c6"
