@ECHO OFF

REM  
REM  Copyright (C) 2005 SIPfoundry Inc.
REM  Licensed by SIPfoundry under the LGPL license.
REM 
REM  Copyright (C) 2005 Pingtel Corp.
REM  Licensed to SIPfoundry under a Contributor Agreement.
REM

SET PROJECT_NAME=%1
SET SUBPROJECT=%2

ECHO .
ECHO PROJECT_NAME: %PROJECT_NAME%
ECHO SUBPROJECT  : %SUBPROJECT%
ECHO .

pushd .

ECHO ??? CD "..\%SUBPROJECT%"
cd "..\%SUBPROJECT%"
IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
ECHO ??? Building...
rmdir /S /Q lib
mkdir lib
call ant -f build-wnt.xml deploy -Dversion=2.6.0 -Dbuild_number=28
ECHO ??? Build result: %ERRORLEVEL%
IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

GOTO DONE

:ERROR_EXIT
ECHO .
ECHO *** %0 Error detected, aborting ... ***
Echo .
popd .
EXIT /B 1

:DONE
popd .
EXIT /B 0
