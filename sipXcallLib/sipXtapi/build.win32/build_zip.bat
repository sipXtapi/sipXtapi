REM @ECHO OFF

REM  
REM  Copyright (C) 2004, 2005 Pingtel Corp.
REM  
REM

IF NOT "%SIPXTAPI_WINZIP_BASE%" == "" GOTO BUILD_IT
SET SIPXTAPI_WINZIP_BASE=C:\Program Files\WinZip

:BUILD_IT

IF "%1" == "CML" GOTO SET_CML
GOTO BUILD_ZIP

:SET_CML
SET TARGET_NAME=sipXtapi_WIN32_%2_%date:~10,4%-%date:~4,2%-%date:~7,2%-CML.zip
GOTO BUILD_ZIP

:BUILD_ZIP
  "%SIPXTAPI_WINZIP_BASE%\wzzip" -rp %TARGET_NAME% staging\
