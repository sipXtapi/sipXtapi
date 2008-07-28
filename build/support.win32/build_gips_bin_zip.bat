@ECHO OFF

CALL setRepoEnvVar.bat

IF NOT "%SIPXTAPI_WINZIP_BASE%" == "" GOTO BUILD_IT
SET SIPXTAPI_WINZIP_BASE=C:\Program Files\WinZip

:BUILD_IT
SET TARGET_NAME="..\gips_WIN32_bin_%1%REPRO_VERSION%_%date:~10,4%-%date:~4,2%-%date:~7,2%.zip"

GOTO BUILD_ZIP

:BUILD_ZIP
  "%SIPXTAPI_WINZIP_BASE%\wzzip" -rp %TARGET_NAME% ..\Build.gips.Win32
