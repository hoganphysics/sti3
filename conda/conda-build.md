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
- To install from a local build (e.g., for development or debugging sti3), run:
```bash
conda install --use-local stipy
```
- Alternatively, to install from Anaconda Cloud, run:
```bash
conda install -c conda-forge hoganlab::stipy
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
