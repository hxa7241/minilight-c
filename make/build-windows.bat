@echo off


rem --- using: MS VC++ 2005 or 2008 ---


mkdir obj
del /Q obj\*
cd obj


rem - set options

set COMPILER=cl
set LINKER=link

rem for x64: maybe add /favor:AMD64 or /favor:INTEL64 to compiler options as appropriate
rem for x64: remove /arch:SSE from compiler options
set COMPILE_OPTIONS=/c /O2 /GL /arch:SSE /fp:fast /GS- /MT /W4 /WL /D_CRT_SECURE_NO_WARNINGS /Isrc


rem - compile and link

@echo.
%COMPILER% %COMPILE_OPTIONS% ../src/*.c

@echo.
%LINKER% /LTCG /OUT:minilight-c.exe kernel32.lib *.obj


move minilight-c.exe ..
cd ..
del /Q obj\*
