@echo off

cd idl\

set files=deviceNet.idl logsNet.idl orbTypes.idl

:: -nf option suppresses "Warning: Forward declared interface '...' was never fully defined"

(for %%file in (%files%) do (
    echo * Parsing %%file
    omniidl -bcxx -Wba -nf -C.\..\src\generated -Wbh=.h -Wbs=.cpp %%file
))

cd ..\src\generated

:: Rename all *.cc to *.cpp
ren *.cc *.cpp
