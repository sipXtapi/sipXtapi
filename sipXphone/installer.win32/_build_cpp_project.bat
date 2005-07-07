@ECHO OFF

REM  
REM  Copyright (C) 2005 SIPfoundry Inc.
REM  Licensed by SIPfoundry under the LGPL license.
REM 
REM  Copyright (C) 2005 Pingtel Corp.
REM  Licensed to SIPfoundry under a Contributor Agreement.
REM

SET PROJECT_NAME=%1

ECHO .
ECHO PROJECT_NAME: %PROJECT_NAME%
ECHO .

pushd .

:BUILD
ECHO ??? Building...
cd ..

msdev %PROJECT_NAME%.dsp /MAKE ALL %2
ECHO ??? Build result: %ERRORLEVEL%
IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

GOTO DONE

:ERROR_EXIT
ECHO .
ECHO *** %0 Error detected, aborting ... ***
popd .
EXIT /B 1

:DONE
popd.
EXIT /B 0
	
