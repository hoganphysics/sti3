//%module(directors="1") example
//%feature("director");

%include "std_string.i"

%rename(opEquals) operator==;
%rename(opLess) operator<;
%rename(opNotEquals) operator!=;
%ignore DeviceIDBase;

%{
    #include "DeviceID.h"
%}

%include "DeviceID.h"

