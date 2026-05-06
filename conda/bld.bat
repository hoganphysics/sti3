@echo on
setlocal EnableExtensions EnableDelayedExpansion


REM Fail fast
IF ERRORLEVEL 1 EXIT 1

set "BUILD_DIR=%SRC_DIR%\_conda_build"
if not defined STI3_CONDA_BUILD_TYPE set "STI3_CONDA_BUILD_TYPE=Release"

if /I "%STI3_CONDA_BUILD_TYPE%"=="Release" goto build_type_ok
if /I "%STI3_CONDA_BUILD_TYPE%"=="RelWithDebInfo" goto build_type_ok
echo STI3_CONDA_BUILD_TYPE must be Release or RelWithDebInfo, got "%STI3_CONDA_BUILD_TYPE%".
exit /b 1

:build_type_ok
set "STI3_INSTALL_PDBS=OFF"
if /I "%STI3_CONDA_BUILD_TYPE%"=="RelWithDebInfo" set "STI3_INSTALL_PDBS=ON"

REM Clean build dir
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"

REM CMake needs to know where to look for deps
set CMAKE_PREFIX_PATH=%PREFIX%;%PREFIX%\\Library
set OPENSSL_ROOT_DIR=%PREFIX%\\Library

REM Generate CORBA stubs from IDL before configuring CMake.
if not exist "%SRC_DIR%\src\network\src\generated" mkdir "%SRC_DIR%\src\network\src\generated"
pushd "%SRC_DIR%\src\network" || exit /b 1
call compileIDL.bat
if errorlevel 1 (
  popd
  exit /b 1
)
popd

REM   -DCMAKE_BUILD_TYPE=Release ^

REM Configure
cmake -S "%SRC_DIR%" -B "%BUILD_DIR%" -G Ninja ^
  -DCMAKE_INSTALL_PREFIX="%PREFIX%" ^
  -DCMAKE_BUILD_TYPE=%STI3_CONDA_BUILD_TYPE% ^
  -DCMAKE_INSTALL_BINDIR=Library\\bin ^
  -DCMAKE_INSTALL_LIBDIR=Library\\lib ^
  -DCMAKE_INSTALL_INCLUDEDIR=Library\\include ^
  -DCMAKE_INSTALL_PYTHONDIR="%SP_DIR%" ^
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL ^
  -DSTI3_INSTALL_PDBS=%STI3_INSTALL_PDBS% ^
  -DSTI3_PACKAGE_BUILD_NUMBER="%PKG_BUILDNUM%" ^
  -DSTI3_BUILD_STRING="%PKG_BUILD_STRING%" ^
  -DSTI3_GIT_COMMIT="%STI3_GIT_COMMIT%" ^
  -DSTI3_GIT_DIRTY="%STI3_GIT_DIRTY%"



IF ERRORLEVEL 1 exit /b 1

REM ----- build (Ninja: no --config) -----
cmake --build "%BUILD_DIR%" -j %CPU_COUNT% || exit /b 1

REM ----- install -----
cmake --install "%BUILD_DIR%" || exit /b 1

endlocal
