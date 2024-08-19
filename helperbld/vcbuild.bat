echo on

set PATH=c:\qt\6.7.2\msvc2019_64\bin;%PATH%

cd "\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build"
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
