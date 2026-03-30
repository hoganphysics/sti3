## Building sti3 and stipy packages with conda-build

- Adjust version numbers in `meta.yaml` files as needed.
- Bump build number in `meta.yaml` file. Otherwise, conda install may skip the package if it detects no changes.
- Activate the conda environment for building if it exists:
```bash
conda activate sti3-build
```
- If it doesn't exist, build and activate a conda environment for building:
```bash
conda create -n sti3-build python=3.13
conda activate sti3-build
conda install -c conda-forge conda-build anaconda-client setuptools pip catch2 cmake ninja
```
- Run the following commands from the project root to build the conda package:
```bash
conda activate sti3-build
conda build -c conda-forge .
```

## Installing the built package
- Switch to the target conda environment where you want to install the package (e.g., `sti3`).
- To install from Anaconda Cloud, run:
```bash
conda install -c conda-forge hoganlab::stipy
```

### Local build installation
- To install from a local build (e.g., for development or debugging sti3), run:
```bash
conda install --use-local stipy
```
- For local installation to work, first create a local channel pointing to the build environment directory:
```bash
conda config --add channels file://$(conda info --base)/envs/sti3-build/conda-bld
```
```cmd
conda config --add channels file:///C:/Users/Jason/miniconda3/envs/sti3-build/conda-bld
```
- If the local build fails to install the newest build, try specifying the exact version and build number explicitly:
```bash
conda install --use-local "stipy=3.0.1=py313h3fd9d12_46"
```
- If that reports already-installed, force reinstall:
```bash
conda install --use-local --force-reinstall stipy
```
- Or remove then install:
```bash
conda remove stipy
conda install --use-local stipy
```

## Uploading the package to Anaconda Cloud
- First, ensure you are logged in to Anaconda Cloud:  
```bash
anaconda login
```
- Upload the built package using the following command:
```bash
anaconda upload /path/to/your/built/package.conda 
```
