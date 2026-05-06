# Legacy Wheel Build Files

This directory archives the old Python wheel build files for STIPy. Wheel builds
are no longer a supported distribution path for this repository; use the conda
package workflow documented in `conda/conda-build.md` instead.

The files here are kept only as historical reference for the former
scikit-build-based wheel packaging flow, including the old setuptools metadata,
build requirements, and source distribution manifest.

The old wheel build wrote generated wheel files to the repository-root `dist/`
directory. That output directory is no longer tracked by the repository.
