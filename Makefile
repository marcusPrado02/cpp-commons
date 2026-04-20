.PHONY: configure build test lint format format-check sanitize clean

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

clean:
	rm -rf build/
