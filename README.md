# STI3

**STI3 (Stanford Timing Interface)** is a C++ and Python toolkit for building
modular, distributed data-acquisition and control systems for physics
experiments. It translates hardware-independent Python timing sequences into
device events and supports both software control and hardware-timed execution.

The repository contains:

- `stidevice`, the core device, event, persistence, logging, and task library;
- `stinetwork`, the CORBA-based RPC and device-network layer;
- `stipy` and `stidevicepy`, the high-level Python API and pybind11 bindings;
- `STIServer`, the network server executable; and
- C++ and Python examples for implementing and exercising devices.

## Build and test

STI3 uses C++20, CMake, and Ninja. Build dependencies include Python development
headers, pybind11, Boost, OpenSSL, omniORB, and Catch2 when tests are enabled.
Create the cross-platform development environment and build with:

```bash
conda env create -f environment-dev.yml
conda activate sti3-build
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_INSTALL_PYTHONDIR=build/Lib/site-packages
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The [`conda/`](conda/) directory contains the conda recipe and platform build
scripts used for packaged builds.

## Examples and documentation

- [`examples/cpp/`](examples/cpp/) contains standalone C++ device examples.
- [`examples/python/`](examples/python/) covers Python devices, event parsing,
  file transfer, logging, monitors, tasks, and post-processing.
- [`test/integration/`](test/integration/) contains the network integration test
  harness.
- [`docs/`](docs/) contains the Sphinx and Doxygen documentation sources and
  build instructions.

## License

STI3 is available under the [MIT License](LICENSE).
