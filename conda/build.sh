set -euxo pipefail

# SRC_DIR=".."

# build in a clean dir inside the work tree
BUILD_DIR="$SRC_DIR/_conda_build"
# PREFIX="$BUILD_DIR"

: "${STI3_CONDA_BUILD_TYPE:=Release}"
case "$STI3_CONDA_BUILD_TYPE" in
  Release|RelWithDebInfo)
    ;;
  *)
    echo "STI3_CONDA_BUILD_TYPE must be Release or RelWithDebInfo, got '$STI3_CONDA_BUILD_TYPE'." >&2
    exit 1
    ;;
esac

STI3_INSTALL_PDBS=OFF
if [ "$STI3_CONDA_BUILD_TYPE" = "RelWithDebInfo" ]; then
  STI3_INSTALL_PDBS=ON
fi

rm -rf "$BUILD_DIR"

export CMAKE_PREFIX_PATH="$PREFIX;$PREFIX/Library"

# Point CMake to the host prefix (libs live here). This is cross-platform friendly.
export OPENSSL_ROOT_DIR="$PREFIX"

mkdir -p "$SRC_DIR/src/network/src/generated"
(
  cd "$SRC_DIR/src/network"
  bash ./compileIDL.sh
)

cmake -S "$SRC_DIR" -B "$BUILD_DIR" -G Ninja \
  -DCMAKE_INSTALL_PREFIX="$PREFIX" \
  -DCMAKE_BUILD_TYPE="$STI3_CONDA_BUILD_TYPE" \
  -DCMAKE_INSTALL_DO_STRIP=OFF \
  -DCMAKE_INSTALL_BINDIR=bin \
  -DCMAKE_INSTALL_LIBDIR=lib \
  -DCMAKE_INSTALL_RPATH="\$ORIGIN/../lib:\$ORIGIN" \
  -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
  -DCMAKE_INSTALL_PYTHONDIR="$SP_DIR" \
  -DSTI3_INSTALL_PDBS="$STI3_INSTALL_PDBS" \
  -DSTI3_PACKAGE_BUILD_NUMBER="$PKG_BUILDNUM" \
  -DSTI3_BUILD_STRING="${PKG_BUILD_STRING:-}" \
  -DSTI3_GIT_COMMIT="${STI3_GIT_COMMIT:-}" \
  -DSTI3_GIT_DIRTY="${STI3_GIT_DIRTY:-}"

# cmake --build "$BUILD_DIR" -j"${CPU_COUNT}"
cmake --build "$BUILD_DIR" -j4

# Issues with rpath in tests, disable for now
# ctest --test-dir "$BUILD_DIR" --output-on-failure

cmake --install "$BUILD_DIR"
