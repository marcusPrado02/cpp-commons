.PHONY: configure build test lint format format-check sanitize coverage docs install fuzz clean

configure:
	cmake --preset=dev

build: configure
	cmake --build --preset=dev --parallel

test: build
	ctest --preset=dev

lint:
	run-clang-tidy -p build/dev

format:
	find include src tests examples \
	  \( -name '*.hpp' -o -name '*.cpp' \) \
	  | xargs clang-format -i

format-check:
	find include src tests examples \
	  \( -name '*.hpp' -o -name '*.cpp' \) \
	  | xargs clang-format --dry-run --Werror

sanitize:
	cmake --preset=dev && cmake --build --preset=dev --parallel
	ctest --preset=dev

coverage:
	cmake --preset=coverage
	cmake --build --preset=coverage --parallel
	ctest --preset=coverage
	lcov --capture --directory build/coverage --output-file build/coverage/coverage.info \
	     --exclude '*/build/*' --exclude '*/_deps/*' --exclude '*/tests/*'
	genhtml build/coverage/coverage.info --output-directory build/coverage/html
	@echo "Coverage report: build/coverage/html/index.html"

docs:
	@command -v doxygen >/dev/null 2>&1 || { echo "doxygen not found — install it first"; exit 1; }
	doxygen Doxyfile
	@echo "Documentation: docs/doxygen/html/index.html"

install:
	cmake --preset=release
	cmake --build --preset=release --parallel
	cmake --install build/release --prefix $(PREFIX)

fuzz:
	@test -n "$(TARGET)" || { echo "Usage: make fuzz TARGET=fuzz_uuid_parse"; exit 1; }
	cmake -B build/fuzz -G Ninja \
	  -DCMAKE_BUILD_TYPE=Debug \
	  -DCMAKE_CXX_FLAGS="-fsanitize=fuzzer,address" \
	  -DCPP_COMMONS_BUILD_TESTS=OFF
	cmake --build build/fuzz --target $(TARGET)
	./build/fuzz/tests/fuzz/$(TARGET)

clean:
	rm -rf build/
