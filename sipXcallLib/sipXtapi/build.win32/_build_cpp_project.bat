@ECHO OFF

REM  
REM  Copyright (C) 2005 SIPfoundry Inc.
REM  Licensed by SIPfoundry under the LGPL license.
REM 
REM  Copyright (C) 2005 Pingtel Corp.
REM  Licensed to SIPfoundry under a Contributor Agreement.
REM

SET PROJECT_NAME=%1


echo build.noerr=1 > ..\..\..\err.prf
ECHO .
ECHO PROJECT_NAME: %PROJECT_NAME%
ECHO .

pushd .

:BUILD
ECHO ??? Building...
cd ..\..

devenv %PROJECT_NAME%.sln /clean debug
devenv %PROJECT_NAME%.sln /build debug
ECHO ??? Build result: %ERRORLEVEL%
IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

devenv %PROJECT_NAME%.sln /clean debug_NoVideo
devenv %PROJECT_NAME%.sln /build debug_NoVideo
ECHO ??? Build result: %ERRORLEVEL%
IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

devenv %PROJECT_NAME%.sln /clean release
devenv %PROJECT_NAME%.sln /build release
ECHO ??? Build result: %ERRORLEVEL%
IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

devenv %PROJECT_NAME%.sln /clean release_NoVideo
devenv %PROJECT_NAME%.sln /build release_NoVideo
ECHO ??? Build result: %ERRORLEVEL%
IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

rem cd ..\..

GOTO DONE

:ERROR_EXIT
ECHO .
ECHO *** %0 Error detected, aborting ... ***
popd .
echo build.err=1 > ..\..\..\err.prf
EXIT /B 1

:DONE
popd.
EXIT /B 0
	
