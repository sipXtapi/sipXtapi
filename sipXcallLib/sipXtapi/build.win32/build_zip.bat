REM @ECHO OFF

REM  
REM  Copyright (C) 2004, 2005 Pingtel Corp.
REM  
REM

IF NOT "%SIPXTAPI_WINZIP_BASE%" == "" GOTO BUILD_IT
SET SIPXTAPI_WINZIP_BASE=C:\Program Files\WinZip

:BUILD_IT
SET TARGET_NAME=sipXtapi_WIN32_%date:~10,4%-%date:~4,2%-%date:~7,2%.zip

:BUILD_ZIP
"%SIPXTAPI_WINZIP_BASE%\wzzip" -rp %TARGET_NAME% staging\
IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

GOTO DONE

:ERROR_EXIT
ECHO .
ECHO *** %0 Error detected, aborting ... ***
ECHO .
exit /b 1

:DONE
exit /b 0
