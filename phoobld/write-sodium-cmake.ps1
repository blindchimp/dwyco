param(
    [string]$SodiumInstall,
    [string]$Sodium
)

if (!(Test-Path $SodiumInstall)) { New-Item -ItemType Directory -Path $SodiumInstall -Force | Out-Null }
if (!(Test-Path "$SodiumInstall\cmake")) { New-Item -ItemType Directory -Path "$SodiumInstall\cmake" -Force | Out-Null }

$sodiumLibRel = "$Sodium\bin\x64\Release\v143\static\libsodium.lib" -replace '\\','/'
$sodiumInc = "$Sodium\src\libsodium\include" -replace '\\','/'

@"
if(TARGET libsodium::libsodium)
  return()
endif()

add_library(libsodium::libsodium STATIC IMPORTED)
set_target_properties(libsodium::libsodium PROPERTIES
  IMPORTED_LOCATION "$sodiumLibRel"
  INTERFACE_INCLUDE_DIRECTORIES "$sodiumInc"
)
"@ | Set-Content -Path "$SodiumInstall\cmake\libsodiumConfig.cmake" -NoNewline

Write-Host "  Created $SodiumInstall\cmake\libsodiumConfig.cmake"
