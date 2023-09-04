#!/bin/bash

cd idl/

files="deviceNet.idl logsNet.idl orbTypes.idl"

# echo ${files}

# -nf option suppresses "Warning: Forward declared interface '...' was never fully defined"

for file in ${files}; do
    echo "* Parsing ${file}"
    omniidl -bcxx -Wba -nf -C./../src/generated -Wbh=.h -Wbs=.cpp ${file}
done

cd ./../src/generated

# Rename all *.cc to *.cpp
for file in *.cc; do
    mv -- "$file" "${file%.cc}.cpp"
done
