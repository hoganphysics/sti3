#!/bin/bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
mkdir -p "${script_dir}/src/generated"
cd "${script_dir}/idl"

files="deviceNet.idl logsNet.idl tasks.idl orbTypes.idl"

# echo ${files}

# -nf option suppresses "Warning: Forward declared interface '...' was never fully defined"

for file in ${files}; do
    echo "* Parsing ${file}"
    omniidl -bcxx -Wba -nf -C./../src/generated -Wbh=.h -Wbs=.cpp ${file}
done

cd ./../src/generated

# Rename all *.cc to *.cpp
for file in *.cc; do
    [ -e "$file" ] || break
    mv -- "$file" "${file%.cc}.cpp"
done
