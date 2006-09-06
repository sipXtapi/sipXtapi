@echo off
@call meta\repo-ver.bat > %TEMP%\repo-ver
IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT

copy meta\repo-ver.txt+%TEMP%\repo-ver  %TEMP%\setit.bat
call %TEMP%\setit.bat
del %TEMP%\setit.bat

GOTO DONE

:ERROR_EXIT
ECHO .
ECHO *** %0 Error detected, aborting ... ***
ECHO .
exit /b 1

:DONE
exit /b 
