# STI network integration tests

This directory is for network/system tests that exercise full simulated STI device networks. These tests are separate from the focused Catch2 unit tests in `test/device/` and `test/network/`.

Current status: initial Python harness. The C++ integration targets are intentionally placeholders.

## Layout

- `python/`: pytest-based integration harness and scenario tests.
- `cpp/`: future C++ integration targets for cases that need lower-level scheduler, network, or lifetime control.

The Python harness supports both in-process topologies and process topologies. `ProcessTopology` runs the server hub in the pytest process and each generated target device in its own child Python process; child devices write JSON-line event records under the topology's temporary persistence root for assertions and diagnostics.

## Intended use

Normal unit-test runs should continue to use the existing CTest/Catch2 flow. Integration tests should be opt-in. Scenarios marked `slow`, `stress`, or `observe` are skipped unless explicitly selected.

Planned Python entry points:

```bash
test/integration/run-python-tests.sh -m "integration and not slow and not observe"
test/integration/run-python-tests.sh -m stress
test/integration/run-python-tests.sh -m observe --observe
```

To attach the smoke test to an existing omniORB name service, pass its address explicitly:

```bash
test/integration/run-python-tests.sh test/integration/python/test_smoke.py --sti-nameservice 192.168.88.252:2809 -q -s
```

Without `--sti-nameservice`, automated tests use `--sti-nameservice-mode=auto`, which spawns a temporary `omniNames` on a non-default local port and removes its data directory after the test. Use `--sti-nameservice-mode=external` with `--sti-nameservice host:port` to attach to an existing service, or `--sti-nameservice-mode=spawn` to force a spawned service.

Observe-mode tests still require an external name service so the frontend can connect with stable settings:

```bash
test/integration/run-python-tests.sh -m observe --observe --sti-nameservice 192.168.88.252:2809 -s
```

The runner defaults to `build` and the `sti3-build` conda environment. It prepends:

- `build/Lib/site-packages` and `test/integration/python` to `PYTHONPATH`;
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
