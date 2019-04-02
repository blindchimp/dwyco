echo on

rem run this in a qt for msvc window

call "c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86

echo on

set D=c:\users\dwight
set SHADOW_NAME=%D%\build-phoo

mkdir %SHADOW_NAME%
rem mkdir $SHADOW_NAME/lib
rem mkdir $SHADOW_NAME/include

rem APPVEYOR_BUILD_FOLDER
cd %SHADOW_NAME%
qmake -v
qmake CONFIG+=release -spec win32-msvc DWYCO_CONFDIR=phoobld %D%\dwyco\phoobld.pro
nmake
