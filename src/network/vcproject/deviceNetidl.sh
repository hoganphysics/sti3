#!/bin/bash

cd ../idl/

omniidl -bcxx -C./../src -Wbh=.h -Wbs=.cpp ./deviceNet.idl

