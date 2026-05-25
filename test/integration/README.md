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
pytest test/integration/python -m "integration and not slow and not observe"
pytest test/integration/python -m stress
pytest test/integration/python -m observe --observe
```

The smoke test needs a running omniORB name service and a Python environment that can import `stipy` plus `pytest`:

```bash
python -m pytest test/integration/python/test_smoke.py --sti-nameservice 192.168.88.252:2809 -q -s
```

When testing directly from `build-ninja` before `stipy` is installed into the active Python environment, add `build-ninja/Lib/site-packages` to `PYTHONPATH` and put the conda environment library directory before system libraries in `LD_LIBRARY_PATH`.
