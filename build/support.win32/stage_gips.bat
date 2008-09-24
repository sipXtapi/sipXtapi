@ECHO OFF

ECHO.
ECHO ****************************************
ECHO Staging gips media binaries
ECHO ****************************************

SET SOURCE_BASE=..\..\sipXmediaVoiceEngine
SET STAGE_BASE=..\build.gips.Win32\bin\
mkdir %STAGE_BASE% 2>nul
mkdir %STAGE_BASE%\Debug 2>nul

:POPULATE_STAGING
  copy %SOURCE_BASE%\Release\sipXmediaLib.dll %STAGE_BASE%\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\Release\sipXmediaLib.pdb %STAGE_BASE%\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\Release\sipXmediaLib.map %STAGE_BASE%\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  
  copy %SOURCE_BASE%\Debug\sipXmediaLib.dll %STAGE_BASE%\Debug\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\Debug\sipXmediaLib.pdb %STAGE_BASE%\Debug\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
  copy %SOURCE_BASE%\Debug\sipXmediaLib.map %STAGE_BASE%\Debug\
  IF NOT "%ERRORLEVEL%"=="0" GOTO ERROR_EXIT
GOTO DONE

:ERROR_EXIT
ECHO.
ECHO ****************************************
ECHO *** %0 Error detected, aborting ... ***
ECHO ****************************************
ECHO .
exit /b 1

:DONE
exit /b 0
