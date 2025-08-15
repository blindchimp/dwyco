echo on

set PATH=c:\qt\6.8.2\msvc2022_64\bin;%PATH%


cd "\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build"
call vcvarsall x64
cd \users\dwight\dwyco
echo on

set D=c:\users\dwight
set SHADOW_NAME=%D%\build-cdcx

mkdir %SHADOW_NAME%
cd %SHADOW_NAME%

%D%\dwyco\cdcx\dumptime %D%\dwyco\cdcx\main.cpp %D%\dwyco\cdcx\buildtime.h


set QTVERS=6.8.2
set QTFLAVOR=msvc2022_64

c:/Qt/%QTVERS%/%QTFLAVOR%/bin/qt-cmake -D QT_QMAKE_EXECUTABLE="%D%/Qt/%QTVERS%/%QTFLAVOR%/bin/qmake" -D CMAKE_BUILD_TYPE=Release -S %D%/dwyco/cdcx -B %D%/build-cdcx -G Ninja

cd %D%/build-cdcx
\Qt\Tools\Ninja\ninja

