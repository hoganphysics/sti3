@echo off
set ipaddress=%1

IF "%ipaddress%"=="" Goto Error

set ORBcommands=-ORBendPointPublish giop:tcp:%ipaddress%:
set logDirectory=.\log


cd .\omninames

:StartOmniNames

omniNames.exe -start 2809 -always -logdir %logDirectory% %ORBcommands%


:Error
pause
exit