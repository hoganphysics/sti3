# Local build directory

## Policy

The top-level `build/` directory is the single CMake build tree for normal
local development and testing. It uses Ninja, the `sti3-build` conda
environment, CMake's `Debug` configuration, and the non-debug multithreaded
DLL runtime on MSVC.

Conda package builds remain isolated in `_conda_build/` inside conda-build's
staged source tree. `docs/build/` and component-local example build directories
are separate workflows and are not substitutes for the top-level tree.

### Conda source staging

The recipe currently uses `source: path: ..`. With conda-build 26.3, local
path sources are copied recursively before the recipe build scripts run, and
that copy does not consult `.condaignore`. As a result, top-level `build/` can
still be copied into conda-build's staged source even though the package is
compiled separately in `_conda_build/`.

Preventing that source copy requires a separate workflow decision: use a local
Git source (committed files only) or maintain an explicit source allowlist. It
should not be conflated with the local CMake build-directory policy.

## Configure, build, and test

```bash
conda run --no-capture-output -n sti3-build cmake \
  -S . \
  -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL \
  -DCMAKE_INSTALL_PREFIX="$PWD/dev" \
  -DCMAKE_INSTALL_PYTHONDIR=build/Lib/site-packages

conda run --no-capture-output -n sti3-build cmake --build build --parallel 8
ctest --test-dir build --output-on-failure
test/integration/run-python-tests.sh --check-env
```

On Linux, if omniORB requires the conda libstdc++ or curl explicitly, add the
linker flags documented in `AGENTS.md` to the configure command.

## Standalone STIServer build

The component-local `src/server/src/build/` tree imports `stidevice` and
`stinetwork` from the top-level `build/` tree. Reconfigure it after changing
the top-level build location or generator:

```bash
conda run --no-capture-output -n sti3-build cmake \
  -S src/server/src \
  -B src/server/src/build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DSTI3_LOCAL_BUILD_DIR:STRING=build

conda run --no-capture-output -n sti3-build cmake \
  --build src/server/src/build --parallel 8
```

The VS Code tasks in the repository root and in `src/server/src/` implement
these same paths.

## Validation

After changing build configuration, verify:

1. The top-level tree configures with Ninja and `CMAKE_BUILD_TYPE=Debug`.
2. MSVC compile commands use `/MD` rather than `/MDd`.
3. The Catch2 suite passes from the top-level tree.
4. The Python integration environment resolves modules and libraries from the
   top-level tree.
5. The standalone server resolves its local STI libraries from the top-level
   tree.
