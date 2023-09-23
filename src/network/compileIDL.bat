@echo off

cd idl\

set files=deviceNet.idl logsNet.idl tasks.idl orbTypes.idl

:: -nf option suppresses "Warning: Forward declared interface '...' was never fully defined"

(for %%f in (%files%) do (
    echo * Parsing %%f
    omniidl -bcxx -Wba -nf -C.\..\src\generated -Wbh=.h -Wbs=.cpp %%f
))

cd ..\src\generated

:: Rename all *.cc to *.cpp
for %%f in (*.cc) do (
    move /Y %%f %%~nf.cpp >nul
)
