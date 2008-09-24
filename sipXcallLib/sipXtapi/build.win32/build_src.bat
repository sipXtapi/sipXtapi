REM @ECHO OFF

IF NOT "%SIPXTAPI_WINZIP_BASE%" == "" GOTO STAGE_IT
SET SIPXTAPI_WINZIP_BASE=C:\Program Files\WinZip

:STAGE_IT
rmdir /S /Q src_stage
svn export https://scm.sipfoundry.org/rep/sipX/branches/sipXtapi src_stage
IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

:BUILD_IT
SET TARGET_NAME=sipXtapi_src_%date:~10,4%-%date:~4,2%-%date:~7,2%.zip
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
