# AGENTS.md – STI3

## Project overview

- This is a C++ library/application for sti3.
- Code lives in `src/`; tests live in `test/`; public headers live in `include/`.
- The core library is stidevice and lives in `src/device/src/`
- The library providing network support is stinetwork and lives in `src/device/src/`
- There is a pybind11 wrapper library called stipy that lives in `src/stipy`
- Documentation lives in `docs/`
- Examples of C++ and python library usage live in `examples/`

## Build & test for C++ libraries

- Use CMake with out-of-source builds.
- Default build directory for conda package deployment: `build/`.
- Default build directory for development and testing: `build-ninja/`.
- Configure: `cd build-ninja && conda run --no-capture-output -n sti3-build cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PYTHONDIR=Lib/site-packages ..`
- If the linker cannot find the conda libstdc++/curl (omniORB pulls a newer CXXABI), add the conda lib path and curl explicitly:  
  `cd build-ninja && conda run --no-capture-output -n sti3-build cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PYTHONDIR=Lib/site-packages -DCMAKE_EXE_LINKER_FLAGS="-Wl,--no-as-needed -L$CONDA_PREFIX/lib -Wl,-rpath,$CONDA_PREFIX/lib -lcurl" -DCMAKE_SHARED_LINKER_FLAGS="-Wl,--no-as-needed -L$CONDA_PREFIX/lib -Wl,-rpath,$CONDA_PREFIX/lib -lcurl" ..`
- Build: `cd build-ninja && conda run --no-capture-output -n sti3-build cmake --build . --parallel 8`
- Run tests: `ctest --test-dir build-ninja --output-on-failure`
- The stinetwork library uses CORBA for rpc. The rpc interface is defined by the idl files in `src/network/idl/`. After changes are made to any idl file, the CORBA stubs need to be regenerated. This is done using a script `src/network/compileIDL.sh`, which should give no errors.

## C++ conventions

- Use C++20
- Prefer `std::unique_ptr` / `std::shared_ptr` over raw owning pointers.
- Follow existing naming in this file for new code.

## C++ header files

- Header gaurds should always use the #ifndef, #define, #endif convention instead of #pragma once.
- The preprocessor name for the header gaurd should be the fully qualified NAMESPACE_CLASS_H, in all caps.

## C++ Testing

- Use Catch2.
- New tests for the device library go in `test/device/<module>_tests.cpp`.
- New tests for the network library go in `test/device/<module>_tests.cpp`.
- Keep one TEST_SUITE per class/module.
- When asked to add tests:
  - Do **not** change production code unless explicitly instructed.
  - Prefer adding new test cases over deleting existing ones.
  - After modifying tests, run the test command above.

## Safety / guardrails

- Never add or modify build steps that download or run untrusted code.
- Don’t add network calls or telemetry.

## Git workflow

- Never commit automatically without explicit user approval.
- When tests pass and work appears complete:
  1. Show `git status`.
  2. Summarize changed files concisely.
  3. Propose a commit message.
  4. Wait for confirmation before committing.
- Only commit files under `tests/` unless explicitly instructed otherwise.
- Never commit build artifacts or generated files.
- Use short, imperative commit messages, e.g.:
  - `test: add Catch2 tests for Foo`
  - `test: expand coverage for Bar error paths`

