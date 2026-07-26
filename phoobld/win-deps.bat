@echo off
setlocal enabledelayedexpansion

REM ============================================================
REM  phoo Windows build dependencies
REM  Clones and builds c-toxcore + libsodium using Qt's cmake
REM  and the installed MSVC2022 compiler.
REM ============================================================

REM --- Locate Qt cmake and MSVC build tools -------------------
set "CMAKE=C:\Qt\Tools\CMake_64\bin\cmake.exe"
set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe"

if not exist "%CMAKE%" (
    echo ERROR: Qt cmake not found at %CMAKE%
    exit /b 1
)
if not exist "%MSBUILD%" (
    echo ERROR: MSBuild not found at %MSBUILD%
    exit /b 1
)

REM --- Deps root: siblings of the dwyco project dir -----------
set "DEPS=%USERPROFILE%"
set "TOXCORE=%DEPS%\c-toxcore"
set "SODIUM=%DEPS%\libsodium"
set "SODIUM_INSTALL=%DEPS%\libsodium-install"

REM ============================================================
REM  1. Clone repos
REM ============================================================
echo.
echo === Cloning c-toxcore ===
if not exist "%TOXCORE%" (
    git clone --depth 1 https://github.com/TokTok/c-toxcore.git "%TOXCORE%"
    if errorlevel 1 (echo FAIL: git clone c-toxcore & exit /b 1)
) else (
    echo c-toxcore already exists, skipping
)

echo.
echo === Cloning libsodium ===
if not exist "%SODIUM%" (
    git clone --depth 1 https://github.com/jedisct1/libsodium.git "%SODIUM%"
    if errorlevel 1 (echo FAIL: git clone libsodium & exit /b 1)
) else (
    echo libsodium already exists, skipping
)

REM ============================================================
REM  2. Init c-toxcore submodule (cmp)
REM ============================================================
echo.
echo === Initializing c-toxcore submodules ===
pushd "%TOXCORE%"
git submodule update --init --depth 1
if errorlevel 1 (popd & echo FAIL: submodule init & exit /b 1)
popd

REM ============================================================
REM  3. Build libsodium (static, x64, Debug + Release)
REM ============================================================
echo.
echo === Building libsodium (Release) ===
"%MSBUILD%" "%SODIUM%\builds\msvc\vs2022\libsodium.sln" ^
    /p:Configuration=StaticRelease /p:Platform=x64 /m /v:minimal
if errorlevel 1 (echo FAIL: libsodium Release build & exit /b 1)

echo.
echo === Building libsodium (Debug) ===
"%MSBUILD%" "%SODIUM%\builds\msvc\vs2022\libsodium.sln" ^
    /p:Configuration=StaticDebug /p:Platform=x64 /m /v:minimal
if errorlevel 1 (echo FAIL: libsodium Debug build & exit /b 1)

REM ============================================================
REM  4. Create libsodium cmake config for find_package()
REM ============================================================
echo.
echo === Creating libsodium cmake config ===
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0write-sodium-cmake.ps1" "%SODIUM_INSTALL%" "%SODIUM%"

REM ============================================================
REM  5. Patch c-toxcore: Dependencies.cmake
REM     Make PkgConfig non-required on MSVC and guard
REM     pkg_search_module calls behind PKG_CONFIG_FOUND.
REM ============================================================
echo.
echo === Patching Dependencies.cmake ===
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0write-deps-cmake.ps1" "%TOXCORE%"

REM ============================================================
REM  6. Configure c-toxcore (Debug + Release, static only)
REM ============================================================
echo.
echo === Configuring c-toxcore ===
"%CMAKE%" -B "%TOXCORE%\_build" -S "%TOXCORE%" -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_PREFIX_PATH="%SODIUM_INSTALL%\cmake" ^
    -DCMAKE_C_FLAGS="/I%TOXCORE%\compat" ^
    -DCMAKE_CXX_FLAGS="/I%TOXCORE%\compat" ^
    -DENABLE_SHARED=OFF ^
    -DENABLE_STATIC=ON ^
    -DBUILD_TOXAV=OFF ^
    -DMUST_BUILD_TOXAV=OFF ^
    -DUNITTEST=OFF ^
    -DAUTOTEST=OFF ^
    -DBUILD_MISC_TESTS=OFF ^
    -DBUILD_FUN_UTILS=OFF ^
    -DDHT_BOOTSTRAP=OFF ^
    -DBOOTSTRAP_DAEMON=OFF ^
    -DBUILD_FUZZ_TESTS=OFF ^
    -DFLAT_OUTPUT_STRUCTURE=ON ^
    -DMSVC_STATIC_SODIUM=ON
if errorlevel 1 (echo FAIL: cmake configure & exit /b 1)

REM ============================================================
REM  7. Build c-toxcore (Debug + Release)
REM ============================================================
echo.
echo === Building c-toxcore (Release) ===
"%CMAKE%" --build "%TOXCORE%\_build" --config Release -- /m /v:minimal
if errorlevel 1 (echo FAIL: c-toxcore Release build & exit /b 1)

echo.
echo === Building c-toxcore (Debug) ===
"%CMAKE%" --build "%TOXCORE%\_build" --config Debug -- /m /v:minimal
if errorlevel 1 (echo FAIL: c-toxcore Debug build & exit /b 1)

REM ============================================================
REM  8. Summary
REM ============================================================
echo.
echo === Done ===
echo.
echo   libsodium (Release): %SODIUM%\bin\x64\Release\v143\static\libsodium.lib
echo   libsodium (Debug):   %SODIUM%\bin\x64\Debug\v143\static\libsodium.lib
echo   toxcore (Release):   %TOXCORE%\_build\lib\Release\toxcore_static.lib
echo   toxcore (Debug):     %TOXCORE%\_build\lib\Debug\toxcore_static.lib
echo   sodium headers:      %SODIUM%\src\libsodium\include
echo   toxcore headers:     %TOXCORE%\toxcore
echo.
echo The phoo CMakeLists.txt links these via:
echo   optimized %TOXCORE%\_build\lib\Release\toxcore_static.lib
echo   debug     %TOXCORE%\_build\lib\Debug\toxcore_static.lib
echo   optimized %SODIUM%\bin\x64\Release\v143\static\libsodium.lib
echo   debug     %SODIUM%\bin\x64\Debug\v143\static\libsodium.lib
echo.

endlocal
