echo on

set PATH=c:\qt\6.8.2\msvc2022_64\bin;%PATH%


cd "\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build"
call vcvarsall x64
cd \users\dwight\dwyco
echo on

set D=c:\users\dwight
set SHADOW_NAME=%D%\build-cdcx

mkdir %SHADOW_NAME%
rem mkdir $SHADOW_NAME/lib
rem mkdir $SHADOW_NAME/include

rem APPVEYOR_BUILD_FOLDER
cd %SHADOW_NAME%
%D%\dwyco\cdcx\dumptime %D%\dwyco\cdcx\main.cpp %D%\dwyco\cdcx\buildtime.h

qmake -v
qmake CONFIG+=release -spec win32-msvc DWYCO_CONFDIR=cdcxbld \users\dwight\dwyco\dwycore.pro
nmake
