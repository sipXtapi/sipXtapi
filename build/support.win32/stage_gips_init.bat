@ECHO OFF

ECHO.
ECHO ****************************************
ECHO Creating Staging Area 
ECHO ****************************************

SET STAGE_BASE=..\build.gips.Win32

:PREP_STAGING
  rmdir /S /Q %STAGE_BASE%
  
  mkdir %STAGE_BASE% 2>NUL
  mkdir %STAGE_BASE%\bin 2>NUL
  mkdir %STAGE_BASE%\lib 2>NUL
  mkdir %STAGE_BASE%\include 2>NUL
  mkdir %STAGE_BASE%\doc 2>NUL

:DONE
exit /b 0
