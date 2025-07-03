echo on

set PATH=c:\qt\6.8.2\msvc2022_64\bin;%PATH%

cd "\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build"
call vcvarsall x64
cd \users\dwight\dwyco
echo on

set D=c:\users\dwight
set SHADOW_NAME=%D%\build-helper

mkdir %SHADOW_NAME%
rem mkdir $SHADOW_NAME/lib
rem mkdir $SHADOW_NAME/include

rem APPVEYOR_BUILD_FOLDER
cd %SHADOW_NAME%
qmake -v
qmake CONFIG+=release -spec win32-msvc DWYCO_CONFDIR=helperbld \users\dwight\dwyco\helpers.pro
nmake release
