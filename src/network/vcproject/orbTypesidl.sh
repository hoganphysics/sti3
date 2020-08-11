#!/bin/bash

cd ../idl/

omniidl -bcxx -C./../src -Wbh=.h -Wbs=.cpp ./orbTypes.idl

