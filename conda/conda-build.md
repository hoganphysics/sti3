## Building sti3 and stipy packages with conda-build

Adjust the version metadata in `sti3_version.json` as needed.

- Bump `version` for a new code release.
- Bump `build_number` for a packaging-only rebuild of the same code release.
- Keep `Release` as the normal user package.
- Use `RelWithDebInfo` for the developer package with native symbols.

Create or update the build environment:

```bash
conda create -n sti3-build python=3.13
conda activate sti3-build
conda install -c conda-forge conda-build anaconda-client setuptools pip catch2 cmake ninja
```

The recipe is controlled by `STI3_CONDA_BUILD_TYPE`. Valid values are `Release` and `RelWithDebInfo`. If it is unset, the recipe builds `Release`.

The build string includes the flavor:

```text
release_py313h..._<build_number>
rdbg_py313h..._<build_number>
```

`rdbg` means `RelWithDebInfo`.

At runtime, this same conda build string is compiled into `STI3_BUILD_STRING`. It is exposed through `STI::Device::getSTILibraryVersion().buildString`, appears in `getSTILibraryVersionSummary()`, and is also stored in version metadata as `conda_build_string`.

## Build Packages

Run these commands from the project root. `conda build .` works because conda-build searches the tree and finds the single recipe at `conda/meta.yaml`. Passing `conda` instead of `.` is equivalent and more explicit.

### Release Package

PowerShell:

```powershell
conda activate sti3-build
$env:STI3_CONDA_BUILD_TYPE = "Release"
conda build -c conda-forge .
Remove-Item Env:\STI3_CONDA_BUILD_TYPE
```

Bash:

```bash
conda activate sti3-build
export STI3_CONDA_BUILD_TYPE=Release
conda build -c conda-forge .
unset STI3_CONDA_BUILD_TYPE
```

### Developer Symbol Package

PowerShell:

```powershell
conda activate sti3-build
$env:STI3_CONDA_BUILD_TYPE = "RelWithDebInfo"
conda build -c conda-forge .
Remove-Item Env:\STI3_CONDA_BUILD_TYPE
```

Bash:

```bash
conda activate sti3-build
export STI3_CONDA_BUILD_TYPE=RelWithDebInfo
conda build -c conda-forge .
unset STI3_CONDA_BUILD_TYPE
```

The developer package installs Windows PDB files beside the matching DLL, EXE, and PYD files. Linux `RelWithDebInfo` builds keep debug information in the binaries unless stripped by downstream packaging tools.

To print the package path without rebuilding, run the matching command with `--output`:

```bash
conda build -c conda-forge . --output
```

## Local Install

Install the newest local build:

```bash
conda install --use-local stipy
```

If conda chooses the wrong flavor, specify the build string pattern:

```bash
conda install --use-local "stipy=*=release_*"
conda install --use-local "stipy=*=rdbg_*"
```

For local installation to work, first create a local channel pointing to the build environment directory.

Bash:

```bash
conda config --add channels file://$(conda info --base)/envs/sti3-build/conda-bld
```

Windows:

```cmd
conda config --add channels file:///C:/Users/Jason/miniconda3/envs/sti3-build/conda-bld
```

If conda reports the package is already installed, force reinstall:

```bash
conda install --use-local --force-reinstall "stipy=*=rdbg_*"
```

## Upload to Anaconda Cloud

Log in first:

```bash
anaconda login
```

Upload the release package to the default `main` label:

```bash
anaconda upload --label main /path/to/stipy-<version>-release_py313h..._<build_number>.conda
```

Upload the developer symbol package to a non-default label:

```bash
anaconda upload --label dev /path/to/stipy-<version>-rdbg_py313h..._<build_number>.conda
```

Do not upload both flavors to `main`. Keeping `rdbg` on the `dev` label makes the release package the default for normal users.

## Install from Anaconda Cloud

Normal users install the release package:

```bash
conda install -c conda-forge -c hoganlab stipy
```

Developers install the symbol package from the `dev` label:

```bash
conda install -c conda-forge -c hoganlab/label/dev "stipy=*=rdbg_*"
```

To switch back to release:

```bash
conda install -c conda-forge -c hoganlab "stipy=*=release_*"
```

## Windows PDB Locations

In the `RelWithDebInfo` developer package, PDB files are installed next to the binary they describe:

```text
%CONDA_PREFIX%\Library\bin\stidevice.pdb
%CONDA_PREFIX%\Library\bin\stinetwork.pdb
%CONDA_PREFIX%\Library\bin\STIServer.pdb
%CONDA_PREFIX%\Lib\site-packages\stipy\stipy.pdb
%CONDA_PREFIX%\Lib\site-packages\stipy\stidevicepy\stidevicepy.pdb
%CONDA_PREFIX%\Lib\site-packages\stipy\stipybase\stipybase.pdb
```

Visual Studio usually finds these automatically because they sit beside the loaded DLL, EXE, or PYD. To add them explicitly, open `Tools > Options > Debugging > Symbols` and add:

```text
%CONDA_PREFIX%\Library\bin
%CONDA_PREFIX%\Lib\site-packages\stipy
%CONDA_PREFIX%\Lib\site-packages\stipy\stidevicepy
%CONDA_PREFIX%\Lib\site-packages\stipy\stipybase
```

In VS Code with the Microsoft C/C++ debugger on Windows, add the same directories to `symbolSearchPath` in the C++ debug configuration:

```json
"symbolSearchPath": "${env:CONDA_PREFIX}\\Library\\bin;${env:CONDA_PREFIX}\\Lib\\site-packages\\stipy;${env:CONDA_PREFIX}\\Lib\\site-packages\\stipy\\stidevicepy;${env:CONDA_PREFIX}\\Lib\\site-packages\\stipy\\stipybase"
```

For Python extension debugging, make sure the active terminal or launch environment has `CONDA_PREFIX` set to the environment where the `rdbg` package is installed.
