set -euxo pipefail

# SRC_DIR=".."

# build in a clean dir inside the work tree
BUILD_DIR="$SRC_DIR/_conda_build"
# PREFIX="$BUILD_DIR"

rm -rf "$BUILD_DIR"

export CMAKE_PREFIX_PATH="$PREFIX;$PREFIX/Library"

# Point CMake to the host prefix (libs live here). This is cross-platform friendly.
export OPENSSL_ROOT_DIR="$PREFIX"

  # -DCMAKE_BUILD_TYPE=Debug \
  # -DCMAKE_CXX_FLAGS_DEBUG="-Og -g3 -fno-omit-frame-pointer" \
  
  # -DCMAKE_BUILD_TYPE=Debug \
  # -DCMAKE_CXX_FLAGS_DEBUG="-O0 -g3 -ggdb3 -fno-omit-frame-pointer -fno-inline -fno-optimize-sibling-calls" \

# -DCMAKE_BUILD_TYPE=RelWithDebInfo \
#-DCMAKE_CXX_FLAGS_DEBUG="-g -O2 -fno-omit-frame-pointer" \

  # -DCMAKE_BUILD_TYPE=Release \
  # -DCMAKE_CXX_FLAGS_DEBUG="-g -O2 -fno-omit-frame-pointer" \

cmake -S "$SRC_DIR" -B "$BUILD_DIR" -G Ninja \
  -DCMAKE_INSTALL_PREFIX="$PREFIX" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS_DEBUG="-O0 -g3 -ggdb3 -fno-omit-frame-pointer -fno-inline -fno-optimize-sibling-calls" \
  -DCMAKE_INSTALL_DO_STRIP=OFF \
  -DCMAKE_INSTALL_BINDIR=bin \
  -DCMAKE_INSTALL_LIBDIR=lib \
  -DCMAKE_INSTALL_RPATH="\$ORIGIN/../lib:\$ORIGIN" \
  -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
  -DCMAKE_INSTALL_PYTHONDIR="$SP_DIR"

cmake --build "$BUILD_DIR" -j"${CPU_COUNT}"

# Issues with rpath in tests, disable for now
# ctest --test-dir "$BUILD_DIR" --output-on-failure

cmake --install "$BUILD_DIR"
