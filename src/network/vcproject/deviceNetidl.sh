#!/bin/bash

cd ../idl/

omniidl -bcxx -Wba -C./../src -Wbh=.h -Wbs=.cpp ./deviceNet.idl

omniidl -bcxx -Wba -C./../src -Wbh=.h -Wbs=.cpp ./logsNet.idl

cd ../src
mv deviceNetDynSK.cc deviceNetDynSK.cpp
mv logsNetDynSK.cc deviceNetDynSK.cpp
