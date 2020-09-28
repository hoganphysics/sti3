#!/bin/bash

cd ../idl/

omniidl -bcxx -Wba -C./../src -Wbh=.h -Wbs=.cpp ./deviceNet.idl

cd ../src
mv deviceNetDynSK.cc deviceNetDynSK.cpp