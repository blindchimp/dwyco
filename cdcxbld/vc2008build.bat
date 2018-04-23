rem NOTE: use this batch file from a "qt-4.8 command prompt"
rem it will set up all the env vars for you
rem

set D=c:\users\dwight
set SHADOW_NAME=%D%\build-cdcx

mkdir %SHADOW_NAME%
rem mkdir $SHADOW_NAME/lib
rem mkdir $SHADOW_NAME/include

set APPVEYOR_BUILD_FOLDER=%D%\cdcxbld
cd %SHADOW_NAME%
qmake CONFIG+=release DWYCO_CONFDIR=cdcxbld %APPVEYOR_BUILD_FOLDER%\dwycore.pro
nmake release
