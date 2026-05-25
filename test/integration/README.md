# STI network integration tests

This directory is for network/system tests that exercise full simulated STI device networks. These tests are separate from the focused Catch2 unit tests in `test/device/` and `test/network/`.

Current status: initial Python harness. The C++ integration targets are intentionally placeholders.

## Layout

- `python/`: pytest-based integration harness and scenario tests.
- `cpp/`: future C++ integration targets for cases that need lower-level scheduler, network, or lifetime control.

## Intended use

Normal unit-test runs should continue to use the existing CTest/Catch2 flow. Integration tests should be opt-in, especially scenarios marked `slow`, `stress`, or `observe`.

Planned Python entry points:

```bash
test/integration/run-python-tests.sh -m "integration and not slow and not observe"
test/integration/run-python-tests.sh -m stress
test/integration/run-python-tests.sh -m observe --observe
```

The smoke test needs a running omniORB name service and a Python environment that can import `stipy` plus `pytest`:

```bash
test/integration/run-python-tests.sh test/integration/python/test_smoke.py --sti-nameservice 192.168.88.252:2809 -q -s
```

The runner defaults to `build-ninja` and the `sti3-build` conda environment. It prepends:

- `build-ninja/Lib/site-packages` and `test/integration/python` to `PYTHONPATH`;
- the selected Python environment's `lib` directory, then the build-tree STI library directories, to `LD_LIBRARY_PATH`.

Useful runner checks:

```bash
test/integration/run-python-tests.sh --print-env
test/integration/run-python-tests.sh --check-env
```

Override defaults with:

- `STI3_BUILD_DIR=/path/to/build`
- `STI3_CONDA_ENV=sti3-build`
- `STI3_CONDA_PREFIX=/path/to/conda/env`
- `STI3_INTEGRATION_PYTHON=/path/to/python`

`pytest` must be installed in the selected Python environment. If the default conda environment can import local `stipy` but not `pytest`, install pytest there or point `STI3_INTEGRATION_PYTHON` at a compatible Python environment.
