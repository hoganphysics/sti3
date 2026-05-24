# STI network integration tests

This directory is for network/system tests that exercise full simulated STI device networks. These tests are separate from the focused Catch2 unit tests in `test/device/` and `test/network/`.

Current status: scaffold only. The Python harness and C++ integration targets are intentionally placeholders until the first harness pass is implemented.

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
