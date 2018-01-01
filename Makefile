 CMAKE_BUILD_TYPE := RelWithDebInfo
#CMAKE_BUILD_TYPE := Debug
CMAKE_BUILD_OPTIONS := -DENABLE_SGX=OFF

build: cmake
	cd build && $(MAKE)

cmake:
	mkdir -p build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ${CMAKE_BUILD_OPTIONS}

test: build
	cd build && $(MAKE) test

test-verbose:
	BOOST_TEST_LOG_LEVEL=message $(MAKE) test ARGS="-VV"

clean:
	-rm -rf build

clang-format:
	@find . -type f \( -name "*.cpp" -or -name "*.h" -or -name "*.hpp" \) \
		-not -path "./build/*" \
		-print -exec clang-format -i {} \;

.PHONY: build clang-format clean cmake test test-verbose
