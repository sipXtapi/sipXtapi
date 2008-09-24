@ECHO OFF
REM Copyright 2008 AOL LLC.
REM Licensed to SIPfoundry under a Contributor Agreement.
REM
REM This library is free software; you can redistribute it and/or
REM modify it under the terms of the GNU Lesser General Public
REM License as published by the Free Software Foundation; either
REM version 2.1 of the License, or (at your option) any later version.
REM
REM This library is distributed in the hope that it will be useful,
REM but WITHOUT ANY WARRANTY; without even the implied warranty of
REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
REM Lesser General Public License for more details.
REM
REM You should have received a copy of the GNU Lesser General Public
REM License along with this library; if not, write to the Free Software
REM Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
REM USA. 
REM  
REM Copyright (C) 2004-2006 SIPfoundry Inc.
REM Licensed by SIPfoundry under the LGPL license.
REM
REM Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
REM Licensed to SIPfoundry under a Contributor Agreement.

CALL setRepoEnvVar.bat

IF NOT "%SIPXTAPI_WINZIP_BASE%" == "" GOTO STAGE_IT
SET SIPXTAPI_WINZIP_BASE=C:\Program Files\WinZip

:STAGE_IT
rmdir /S /Q ..\src.win32
svn export %1 ..\src.win32
IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT 

:BUILD_IT
SET TARGET_NAME=..\sipXtapi_WIN32_src_%2%REPRO_VERSION%_%date:~10,4%-%date:~4,2%-%date:~7,2%.zip
"%SIPXTAPI_WINZIP_BASE%\wzzip" -rp %TARGET_NAME% ..\src.win32\
IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

GOTO DONE

:ERROR_EXIT
ECHO .
ECHO *** %0 Error detected, aborting ... ***
ECHO .
exit /b 1

:DONE
exit /b 0
