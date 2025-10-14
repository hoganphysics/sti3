@echo on
setlocal EnableExtensions EnableDelayedExpansion


REM Fail fast
IF ERRORLEVEL 1 EXIT 1

set "BUILD_DIR=%SRC_DIR%\_conda_build"

REM Clean build dir
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"

REM CMake needs to know where to look for deps
set CMAKE_PREFIX_PATH=%PREFIX%;%PREFIX%\\Library
set OPENSSL_ROOT_DIR=%PREFIX%\\Library

REM   -DCMAKE_BUILD_TYPE=Release ^

REM Configure
cmake -S "%SRC_DIR%" -B "%BUILD_DIR%" -G Ninja ^
  -DCMAKE_INSTALL_PREFIX="%PREFIX%" ^
  -DCMAKE_BUILD_TYPE=RelWithDebInfo ^
  -DCMAKE_CXX_FLAGS_DEBUG="-g -O2 -fno-omit-frame-pointer" ^
  -DCMAKE_INSTALL_BINDIR=Library\\bin ^
  -DCMAKE_INSTALL_LIBDIR=Library\\lib ^
  -DCMAKE_INSTALL_INCLUDEDIR=Library\\include ^
  -DCMAKE_INSTALL_PYTHONDIR="%SP_DIR%" ^
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL



IF ERRORLEVEL 1 exit /b 1

REM ----- build (Ninja: no --config) -----
cmake --build "%BUILD_DIR%" -j %CPU_COUNT% || exit /b 1

REM ----- install -----
cmake --install "%BUILD_DIR%" || exit /b 1

endlocal
