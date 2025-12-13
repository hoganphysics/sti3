# test-specific rules

## Building and running tests

- Use Catch2 for C++ tests; register tests via catch_discover_tests in test/CMakeLists.txt.
- Build tests in the build-ninja out-of-source dir and run with ctest --test-dir build-ninja --output-on-failure after conda run ... cmake --build ..


## File structure and naming convention

- Follow file naming test/device/<module>_tests.cpp (and analogous for other areas) with one TEST_SUITE/module.
- Tests for the device library go in `test/device/`
- Tests for the network library go in `test/network/`
- Each cpp file we test from `src/` gets its own test cpp file.


## Test coverage

- Add focused Catch2 unit tests per class/module.
- Add tests covering constructors, core behaviors, edge cases, and comparison/serialization paths.
- Aim for meaningful coverage of public APIs with lightweight dummy types where dependencies are heavy.
- Avoid altering production code when adding tests; prefer new test cases over modifying behavior.


## Dummy duplication

- For generic dummy classes that are used for tests in multiple files, reuse the code in a support header file.
- Put common dummy and support classes in a file named: `<module>_tests_support.h`
- Don’t copy-paste the same dummy classes into every test file
-- If it’s used in 2+ test files, move it to support.
-- If it’s used in 1 file, keep it local.
