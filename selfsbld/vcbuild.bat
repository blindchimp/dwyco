echo on
set PATH=c:\qt\5.15.2\msvc2019\bin;%PATH%

rem run this in a qt for msvc window

cd "\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build"
call vcvarsall x86
cd \users\dwight\dwyco

set D=c:\users\dwight
set SHADOW_NAME=%D%\build-selfs

mkdir %SHADOW_NAME%
rem mkdir $SHADOW_NAME/lib
rem mkdir $SHADOW_NAME/include

rem APPVEYOR_BUILD_FOLDER
cd %SHADOW_NAME%
qmake -v
qmake CONFIG+=release -spec win32-msvc DWYCO_CONFDIR=selfsbld %D%\dwyco\selfsbld.pro
nmake
