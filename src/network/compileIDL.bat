@echo off
setlocal EnableExtensions

if not exist "%~dp0src\generated" mkdir "%~dp0src\generated"
pushd "%~dp0idl" || exit /b 1

set files=deviceNet.idl logsNet.idl tasks.idl orbTypes.idl

:: -nf option suppresses "Warning: Forward declared interface '...' was never fully defined"

(for %%f in (%files%) do (
    echo * Parsing %%f
    omniidl -bcxx -Wba -nf -C.\..\src\generated -Wbh=.h -Wbs=.cpp %%f
    if errorlevel 1 (
        popd
        exit /b 1
    )
))

cd ..\src\generated

:: Rename all *.cc to *.cpp
for %%f in (*.cc) do (
    move /Y %%f %%~nf.cpp >nul
    if errorlevel 1 (
        popd
        exit /b 1
    )
)

popd
endlocal
