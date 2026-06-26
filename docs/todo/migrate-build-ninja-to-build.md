# Migrate local builds from build-ninja to build

## Goal

Use the top-level `build/` directory as the single local development CMake build
tree, and retire the top-level `build-ninja/` directory.

This should not change conda package builds. Conda package builds use
`_conda_build/` through `conda/build.sh` and `conda/bld.bat`.

## Current state

- Top-level `build-ninja/` is the active local development build tree.
- Top-level `build/` has been cleaned and currently only preserves its tracked
  `.gitignore`.
- `src/server/src/build/` is the standalone unoptimized `STIServer` debug build.
- `src/server/src/build-local/` is the old standalone debug build directory and
  should remain until debugging from `src/server/src/build/` is verified.
- `docs/build/` is unrelated and remains the docs build output directory.
- `_conda_build/` is unrelated and remains the conda package build directory.

## Target state

- Top-level `build/` is configured with Ninja and the `sti3-build` conda
  environment for local development.
- All tracked local workflow references use top-level `build/` instead of
  top-level `build-ninja/`.
- `src/server/src/build/` links against libraries from top-level `build/src/...`.
- Top-level `build-ninja/` is deleted after validation.
- `src/server/src/build-local/` is deleted after debugger validation.

## Migration steps

1. Commit the current IDL/build cleanup changes before starting this migration.

2. Configure a fresh top-level `build/` tree:

   ```bash
   conda run --no-capture-output -n sti3-build cmake -S . -B build -G Ninja \
     -DCMAKE_BUILD_TYPE=RelWithDebInfo \
     -DCMAKE_INSTALL_PREFIX="$PWD/build/install" \
     -DCMAKE_INSTALL_PYTHONDIR=Lib/site-packages \
     -DCMAKE_EXE_LINKER_FLAGS="-Wl,--no-as-needed -L/home/hogan/miniconda3/envs/sti3-build/lib -Wl,-rpath,/home/hogan/miniconda3/envs/sti3-build/lib -lcurl" \
     -DCMAKE_SHARED_LINKER_FLAGS="-Wl,--no-as-needed -L/home/hogan/miniconda3/envs/sti3-build/lib -Wl,-rpath,/home/hogan/miniconda3/envs/sti3-build/lib -lcurl"
   ```

3. Build and test the top-level `build/` tree:

   ```bash
   conda run --no-capture-output -n sti3-build cmake --build build --parallel 8
   ctest --test-dir build --output-on-failure
   test/integration/run-python-tests.sh --check-env
   ```

4. Update tracked references from `build-ninja` to `build` in active workflow
   files:

   - `AGENTS.md`
   - `CLAUDE.md`
   - `.vscode/tasks.json`
   - `test/AGENTS.md`
   - `test/integration/run-python-tests.sh`
   - `test/integration/python/test_stidevicepy_binary_payloads.py`
   - `test/integration/README.md`

5. Update standalone `STIServer` debug wiring:

   - `src/server/src/CMakeLists.txt`
     - Change the default `STI3_LOCAL_BUILD_DIR` from top-level `build-ninja`
       to top-level `build`.
   - `src/server/src/.vscode/tasks.json`
     - Build local `stidevice` and `stinetwork` from `../../../build`.
     - Configure with `-DSTI3_LOCAL_BUILD_DIR=${workspaceFolder}/../../../build`.
   - `src/server/src/.vscode/launch.json`
     - Change `LD_LIBRARY_PATH` entries from `../../../build-ninja/src/...`
       to `../../../build/src/...`.
   - `src/server/src/notes.txt`
     - Update examples from `build-ninja` to `build`.

6. Reconfigure or rebuild the standalone server debug build:

   ```bash
   conda run --no-capture-output -n sti3-build cmake --build src/server/src/build --parallel 8
   ldd src/server/src/build/STIServer | rg 'stinetwork|stidevice'
   ```

   The `ldd` output should resolve:

   - `libstinetwork.so` from top-level `build/src/network/src`
   - `libstidevice.so` from top-level `build/src/device/src`

7. Re-run local workflow checks after reference changes:

   ```bash
   conda run --no-capture-output -n sti3-build cmake --build build --parallel 8
   ctest --test-dir build --output-on-failure
   test/integration/run-python-tests.sh --check-env
   conda run --no-capture-output -n sti3-build cmake --build src/server/src/build --parallel 8
   ```

8. Delete ignored obsolete build trees only after validation:

   - `build-ninja/`
   - `src/server/src/build-local/`

## Rollback

- If the top-level migration fails, keep using `build-ninja/` and revert the
  tracked reference changes.
- If `STIServer` debugging fails after switching to top-level `build/`, point
  `src/server/src/.vscode/tasks.json` and `launch.json` back to `build-ninja`
  temporarily while debugging the standalone build configuration.
- Do not remove `build-ninja/` until the new top-level `build/` passes local
  build, unit test, integration environment, and standalone `STIServer` checks.

## Cleanup follow-ups

- Keep `build-ninja/` in `.gitignore` during the transition so old commands do
  not accidentally create trackable artifacts.
- After migration settles, decide whether to remove `build-ninja/` from
  `.gitignore`.
- Consider removing tracked generated CORBA files after the new IDL CMake rule
  has been proven reliable from a clean build.
