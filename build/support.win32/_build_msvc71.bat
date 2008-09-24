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

SET PROJECT_NAME=%1

pushd ..\..

:BUILD
ECHO.
ECHO ****************************************
ECHO BUILDING DEBUG: %PROJECT_NAME%
ECHO ****************************************
devenv %PROJECT_NAME%.sln /clean debug
devenv %PROJECT_NAME%.sln /build debug
ECHO ??? Build result: %ERRORLEVEL%
IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

ECHO.
ECHO ****************************************
ECHO BUILDING RELEASE: %PROJECT_NAME%
ECHO ****************************************
devenv %PROJECT_NAME%.sln /clean release
devenv %PROJECT_NAME%.sln /build release
ECHO ??? Build result: %ERRORLEVEL%
IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT


GOTO DONE

:ERROR_EXIT
ECHO.
ECHO ****************************************
ECHO *** %0 Error detected, aborting ... ***
ECHO ****************************************
EXIT /B 1

:DONE
popd.
EXIT /B 0
	
