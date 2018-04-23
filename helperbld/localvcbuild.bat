echo on

set PATH=c:\users\dwight\qt-4.8.7\bin;%PATH%

call "c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86

echo on

set D=c:\users\dwight
set SHADOW_NAME=%D%\build-helper

mkdir %SHADOW_NAME%
rem mkdir $SHADOW_NAME/lib
rem mkdir $SHADOW_NAME/include

set APPVEYOR_BUILD_FOLDER=c:\users\dwight\helperbld
cd %SHADOW_NAME%
qmake CONFIG+=release -spec win32-msvc2015 %APPVEYOR_BUILD_FOLDER%\helpers.pro
nmake release
