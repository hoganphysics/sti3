cls

::set pythonpath=C:\Users\Jason\Code\python\Python-2.5.5\Lib


cd ..\src\corbatest\


omniidl -bcxx -C.\. -Wbh=.h -Wbs=.cpp .\deviceNet.idl

pause