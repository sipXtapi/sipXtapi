REM @ECHO OFF

CALL setRepoEnvVar.bat

IF NOT "%SIPXTAPI_WINZIP_BASE%" == "" GOTO BUILD_IT
SET SIPXTAPI_WINZIP_BASE=C:\Program Files\WinZip

:BUILD_IT
SET TARGET_NAME=sipXtapi_SRC_r%REPRO_VERSION%_%date:~10,4%-%date:~4,2%-%date:~7,2%.zip
"%SIPXTAPI_WINZIP_BASE%\wzzip" -rp %TARGET_NAME% src_stage\
IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

GOTO DONE

:ERROR_EXIT
ECHO .
ECHO *** %0 Error detected, aborting ... ***
ECHO .
exit /b 1

:DONE
exit /b 0
