@echo on
REM Fail fast
IF ERRORLEVEL 1 EXIT 1

REM Clean build dir
if exist "%SRC_DIR%\\_conda_build" rmdir /s /q "%SRC_DIR%\\_conda_build"
mkdir "%SRC_DIR%\\_conda_build"

REM CMake needs to know where to look for deps
set CMAKE_PREFIX_PATH=%PREFIX%;%PREFIX%\\Library
set OPENSSL_ROOT_DIR=%PREFIX%\\Library

REM Configure
cmake -S "%SRC_DIR%" -B "%SRC_DIR%\\_conda_build" -G Ninja ^
  -DCMAKE_INSTALL_PREFIX="%PREFIX%" ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_INSTALL_BINDIR=Library\\bin ^
  -DCMAKE_INSTALL_LIBDIR=Library\\lib ^
  -DCMAKE_INSTALL_INCLUDEDIR=Library\\include ^
  -DCMAKE_INSTALL_PYTHONDIR="%SP_DIR%"

REM Build
cmake --build "%SRC_DIR%\\_conda_build" --config Release -- -j %CPU_COUNT%

REM Install
cmake --install "%SRC_DIR%\\_conda_build" --config Release
