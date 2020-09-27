#!/bin/bash

cd ../idl/

omniidl -bcxx -Wba -C./../src -Wbh=.h -Wbs=.cpp ./orbTypes.idl

cd ../src
mv orbTypesDynSK.cc orbTypesDynSK.cpp