@ECHO OFF

REM  
REM  Copyright (C) 2005 SIPfoundry Inc.
REM  Licensed by SIPfoundry under the LGPL license.
REM 
REM  Copyright (C) 2005 Pingtel Corp.
REM  Licensed to SIPfoundry under a Contributor Agreement.
REM

IF NOT "%SIPXTAPI_WINZIP_BASE%" == "" GOTO BUILD_ZIP
SET SIPXTAPI_WINZIP_BASE=C:\Program Files\WinZip

SET TARGET_NAME=sipXtapi_WIN32_%date:~10,4%-%date:~4,2%-%date:~7,2%.zip

:BUILD_ZIP
  "%SIPXTAPI_WINZIP_BASE%\wzzip" -rp %TARGET_NAME% staging\
