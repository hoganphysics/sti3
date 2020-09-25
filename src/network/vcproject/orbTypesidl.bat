cls

::set pythonpath=C:\Users\Jason\Code\python\Python-2.5.5\Lib


cd ..\idl\


omniidl -bcxx -Wba -C.\..\src -Wbh=.h -Wbs=.cpp .\orbTypes.idl

cd ..\src
move orbTypesDynSK.cc orbTypesDynSK.cpp

::pause